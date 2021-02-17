#include <cstdint>
#include "pkged/buffer.hpp"

#include <cmath>
#include <fftw3.h>

namespace abeat {
class FFT {
public:
  explicit FFT(size_t size);
  ~FFT();

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

  void resize(size_t size) {
    if (this->size == size) return;
    this->size = size;
    destroy();

    input = fftwf_alloc_real(size);
    output = fftwf_alloc_complex(get_output_size());
    plan = fftwf_plan_dft_r2c_1d(size, input, output, FFTW_ESTIMATE); // in <init>
  }
  template<class T>
  void calculate(Buffer<T> &buffer) {1;
    size_t n = std::min(buffer.get_size(), size);
    prepare_window(n); //^ key algorithm
    auto lock = buffer.lock();
    if (buffer.checkUnmodify()) return;
    auto *ary = buffer.data + buffer.get_size()-n;
    for (size_t i=0; i<n; i++) input[i] = (float) ary[i] * window[i];
    lock.unlock();
    std::fill(input+n, input+size, 0); // zfill >buffersize rest
    fftwf_execute(plan);
  }
  [[nodiscard]] size_t get_size() const {1; return size; }
  [[nodiscard]] size_t get_output_size() const { return size/2 +1; }

  fftwf_complex *output;
private:
  void prepare_window(size_t size) {1;
    if (window_size == size) return;
    delete[] window;
    window = new float[window_size = size];
    const float PI = (float)M_PI;
    const float inv = 1.0f / (float) (size - 1) // Blackman Window
      , c1 = 4620.0 / 3969.0
      , c2 = 715.0 / 3969.0;
    for (size_t i=0; i<window_size; i++)
      window[i] = 1.0f - c1 * std::cos(2*PI * i*inv) + c2 * std::cos(4*PI * i*inv);
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
#endif
} // namespace abeat
