#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <cmath>
#include <condition_variable>

#include "config.hpp"
#include "pkged/FFT.hpp"
#include "pkged/gl_render.hpp"
#include "pkged/sndin_pulseaudio.hpp"

GLFWwindow* appWin();
void app(GLFWwindow* win);
int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize glfw" << std::endl;
    return -1;
  }
  auto win = appWin();
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  abeat::config::rebind();
  if (win) app(win);
  glfwTerminate();
  return 0;
}

std::atomic<int> frame_counter(0);
std::condition_variable cv; // at frame sync
std::mutex mutex; // used before fps_printer loop
inline float cdistance(const fftwf_complex &c) { return std::hypot(c[0], c[1]); }

void app(GLFWwindow* win) {
  using namespace abeat;
  bool running; int restart = 0;
  auto buffer = std::make_shared<Buffer<int16_t>>(config::buffer_size); // sample buffer
  PAInput input(buffer); FFT fft(config::window_size); Render render(config::O);

  pa_sample_spec sample_spec = { .format = PA_SAMPLE_S16LE, .rate = config::freq_sample, .channels = config::channels };
  input.spec = &sample_spec;
  input.start(); running = true;
#define test_unbreak ({ std::unique_lock<std::mutex> lock(mutex); !cv.wait_for(lock, std::chrono::operator""s(1), [&]() { return !running; }); })
    std::thread fps_printer([&running]() { // CLI: rate sleep 1s, (!wait_for {return !p}) used
      while (test_unbreak) {
        const int cnt = frame_counter;
        frame_counter = 0;
        std::cout << cnt << "fps" << std::flush << '\r';
      }
    });
    std::thread cfg_changer([&restart, &running]() {
      while (test_unbreak) {
        std::string name, v;
        std::cin >> name >> v;
        setenv(name.c_str(), v.c_str(), true);
        running = false; restart = (name=="hasTransparent"||name=="noTitle")? 2 : 1;
        abeat::config::rebind();
      }
    });
#undef test_unbreak
  {
    using namespace std::chrono;
    time_point<steady_clock> cur_time, last_time;
    const size_t scale = std::min(fft.get_size(), buffer->get_size()); // size given in ctor
    while (running && !glfwWindowShouldClose(win)) { // main: framerate limit dt loop
      cur_time = steady_clock::now();
      std::chrono::duration<float> dt = (cur_time - last_time);
      fft.calculate(*buffer);
      for (size_t i = 0; i < config::O; ++i) {
        const float lo = config::kLo, hi = config::kHi;
        const float dB = (std::clamp(20 * std::log10(cdistance(fft.output[i]) / scale), lo, hi) - lo) / (hi - lo); // key equation
        render.data[i] = dB;
      }
      render.render(dt.count());
      glfwSwapBuffers(win);
      glfwPollEvents();
      ++frame_counter;
      std::this_thread::sleep_until(cur_time + microseconds(1000*SEC_MS/config::max_fps - 100));
      last_time = cur_time;
    }
    {
      std::unique_lock<std::mutex> lock(mutex);
      running = false;
      cv.notify_one(); // break test only
    }
    cfg_changer.detach(); // cause async cin is not possible
    fps_printer.join();
  } // using chrono
  if (restart == 2) { glfwDestroyWindow(win); win = appWin(); }
  if (restart != 0) app(win);
}
GLFWwindow* appWin() {
  glfwWindowHint(GLFW_DECORATED, !abeat::config::noTitle);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, abeat::config::hasTransparent);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // NOTE core?
  auto win = glfwCreateWindow(1000, 350, "abeat", nullptr, nullptr); // initial winsize
  if (!win) {
    std::cerr << "Failed to open GLFW window" << std::endl;
    return nullptr;
  }
  glfwMakeContextCurrent(win);
  glfwSetFramebufferSizeCallback(win, [](GLFWwindow *, int width, int height) {
    glViewport(0, 0, width, height);
  });
  return win;
}
