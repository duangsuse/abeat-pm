#pragma once
#include <cstdint>

namespace abeat::config {

const uint32_t
    freq_sample = 44100
    , latency = 1100
    , max_fps = 60
    , duration = 50;
const uint64_t buffer_size = freq_sample * duration / 1000;
const bool stereo = false;

} // namespace abeat::config