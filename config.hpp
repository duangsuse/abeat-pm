#pragma once
#include <cstdint>
#include <cstdlib>

namespace abeat {
const int SEC_MS = 1000;
namespace config {

uint32_t
    freq_sample = 44100
    , latency = 1100
    , max_fps = 60
    , buf_duration = 50
    , O = 500
    , kLo = 10, kHi = 70;
uint64_t
    buffer_size = freq_sample * buf_duration / SEC_MS
    , window_size = 4096;
uint8_t channels = 1; // support stereo?
bool noTitle = false, hasTransparent = true;

template<typename T> void bindEnv(const char* name, T& vref) { auto v=getenv(name); if (v!=nullptr) vref = (T)atol(v); }
void rebind() {
  bindEnv("samplerate", freq_sample);
  bindEnv("latency", latency);
  bindEnv("maxFPS", max_fps);
  bindEnv("bufDuration", buf_duration);
  bindEnv("windowsize", window_size);
  bindEnv("noTitle", noTitle);
  bindEnv("hasTransparent", hasTransparent);
  bindEnv("O", O);
}

} // namespace abeat::config
} // namespace abeat
