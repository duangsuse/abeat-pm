#include <cstdint>
#include "pkged/buffer.hpp"

#include <cmath>
#include <fftw3.h>

namespace abeat {
class FFT {
public:
  explicit FFT(size_t size);
  FFT(const FFT &) = delete;
#ifdef PKGED
  FFT(FFT &&) noexcept;
#else
  FFT(FFT &&t) noexcept:
    input(t.input), output(t.output),
    window(t.window), size(t.size),
    window_size(t.window_size) {
        t.input = nullptr;
        t.output = nullptr;
        t.plan = nullptr;
        t.window = nullptr;
        t.size = t.window_size = 0; // move
    };
#endif
  FFT &operator=(FFT &&) = default;
  ~FFT();

  void resize(size_t size) {
    if (this->size == size) return;
    this->size = size;
    destroy();

    input = fftwf_alloc_real(size);
    output = fftwf_alloc_complex(get_output_size());
    plan = fftwf_plan_dft_r2c_1d(size, input, output, FFTW_ESTIMATE);
  }
  template<class T>
  void calculate(Buffer<T> &buffer) {1;
    const auto ws = std::min(size, buffer.get_size());
    prepare_window(ws);
    auto lock = buffer.lock();
    if (!buffer.has_new_data) return;
    buffer.has_new_data = false;
    auto *arr = buffer.data + buffer.get_size() - ws;
    for (size_t i = 0; i < ws; ++i)
        input[i] = (float) arr[i] * window[i];
    lock.unlock();
    std::fill(input + ws, input + size, 0);
    fftwf_execute(plan);
  }
  [[nodiscard]] size_t get_size() const;
  [[nodiscard]] size_t get_output_size() const { return size / 2 + 1; }

  fftwf_complex *output;
private:
  void prepare_window(size_t size) {1;
    if (window_size == size) return;
    delete[] window;
    window = new float[window_size = size];
    const float inv = 1.0f / (float) (size - 1)
      , c1 = 4620.0 / 3969.0
      , c2 = 715.0 / 3969.0;
    // Blackman Window
    for (size_t i = 0; i < window_size; ++i)
      window[i] = 1.f - c1 * std::cos(2 * (float) M_PI * i * inv) + c2 * std::cos(4 * (float) M_PI * i * inv);
  }
  void destroy() {
    if (plan) fftwf_destroy_plan(plan);
    if (input) fftwf_free(input);
    if (output) fftwf_free(output);
    delete[] window;
  }

  float *input, *window;
  fftwf_plan plan;
  size_t size, window_size;
};
#ifndef PKGED
FFT::~FFT() { destroy(); }
FFT::FFT(size_t size):
  input(nullptr), output(nullptr),
  plan(nullptr), window(nullptr) { resize(size); }
size_t FFT::get_size() const { return size; }
#endif
} // namespace abeat
