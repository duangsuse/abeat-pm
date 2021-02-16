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

std::atomic<int> frame_counter(0);
std::condition_variable cv;
std::mutex mutex;
bool running;

inline float length(const fftwf_complex &cpl) {
	return std::hypot(cpl[0], cpl[1]);
}
int main() {
	if (!glfwInit()) {
		std::cerr << "Failed to initialize glfw" << std::endl;
		return -1;
	}
	// glfwWindowHint(GLFW_DECORATED, false);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, 1);
	GLFWwindow *window = glfwCreateWindow(1000, 350, "abeat", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to open GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
	});
	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	{
		using namespace std::chrono;
		using namespace abeat;

		const int O = 500;
		Render render(O);
		time_point<steady_clock> cur_time, last_time;
		running = true;
		std::thread fps_printer([]() { // cli
			while (({
				std::unique_lock<std::mutex> lock(mutex);
				!cv.wait_for(lock, 1s, []() { return !running; });
			})) {
				const int cnt = frame_counter;
				frame_counter = 0;
				std::cout << cnt << "fps" << std::flush << '\r';
			}
		});
		auto buffer = std::make_shared<Buffer<int16_t>>(config::buffer_size);
		FFT fft(4096);
		PAInput input(buffer);
		input.start();
		const size_t scale = std::min(fft.get_size(), buffer->get_size());
		while (!glfwWindowShouldClose(window)) {
			cur_time = steady_clock::now();
			std::chrono::duration<float> dt = cur_time - last_time;
			fft.calculate(*buffer);
			for (size_t i = 0; i < O; ++i) {
				const float lo = 10, hi = 70;
				const float dB = (std::clamp(20 * std::log10(length(fft.output[i]) / scale), lo, hi) - lo) / (hi - lo);
				render.data[i] = dB;
			}
			render.render(dt.count());
			glfwSwapBuffers(window);
			glfwPollEvents();
			++frame_counter;
			std::this_thread::sleep_until(
				cur_time + microseconds(1000 * 1000 / config::max_fps - 100));
			last_time = cur_time;
		}
		{
			std::unique_lock<std::mutex> lock(mutex);
			running = false;
			cv.notify_one();
		}
		fps_printer.join();
	}
	glfwTerminate();
	return 0;
}
