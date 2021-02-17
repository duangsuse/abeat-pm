#pragma once
#include <cstdint>

namespace abeat {
const int SEC_MS = 1000;
namespace config {

const uint32_t
    freq_sample = 44100
    , latency = 1100
    , max_fps = 60
    , buf_duration = 50
    , O = 500
    , kLo = 10, kHi = 70;
const uint64_t
    buffer_size = freq_sample * buf_duration / SEC_MS
    , window_size = 4096;
const uint8_t channels = 1; // support stereo?

} // namespace abeat::config
} // namespace abeat
#define DEFINE_LOCK(id, type) struct id##Lock { \
	explicit id##Lock(type *inst): o(inst) { type##_lock(inst); } \
	inline ~id##Lock() { if (o) type##_unlock(o); } \
	inline void unlock() { if (o) { type##_unlock(o); o = nullptr; } } \
private: type *o; };
