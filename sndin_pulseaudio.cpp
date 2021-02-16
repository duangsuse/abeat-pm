#ifndef PKGED
#  include "types.hpp"
#else
#  include "../types.hpp"
#endif
#include "config.hpp"
#include "pkged/buffer.hpp"

#include <pulse/pulseaudio.h>
namespace abeat {
#ifndef PKGED
DEFINE_LOCK(pa_threaded_mainloop)
#endif
struct PAInput {
public:
  explicit PAInput(SPtr<Buffer<int16_t>> buffer);
  PAInput(PAInput&) = delete;
  ~PAInput() {1;
    stop();
    pa_threaded_mainloop_stop(mainloop);
    pa_context_disconnect(context);
    pa_context_unref(context);
    pa_threaded_mainloop_free(mainloop);
  }

  void stop(); void start();
private:
  static void context_state_callback(pa_context *context, void *userdata) {
    auto *that = (PAInput *) userdata;
    switch (pa_context_get_state(context)) {
        case PA_CONTEXT_READY:
        case PA_CONTEXT_TERMINATED:
        case PA_CONTEXT_FAILED:
            pa_threaded_mainloop_signal(that->mainloop, 0);
            break;
        default:
            break;
    }
  }
  static void context_info_callback(pa_context *context, const pa_server_info *info, void *userdata) {
    auto *that = (PAInput *) userdata;
    that->device = info->default_sink_name;
    that->device += ".monitor";
    pa_threaded_mainloop_signal(that->mainloop, 0);
  }
  static void stream_state_callback(pa_stream *stream, void *userdata) {
    auto *that = (PAInput *) userdata;
    switch (pa_stream_get_state(stream)) {
        case PA_STREAM_READY:
        case PA_STREAM_TERMINATED:
        case PA_STREAM_FAILED:
            pa_threaded_mainloop_signal(that->mainloop, 0);
            break;
        default:
            break;
    }
  }
  static void stream_read_callback(pa_stream *stream, size_t length, void *userdata) {
    auto *that = (PAInput *) userdata;
    while (pa_stream_readable_size(stream) > 0) {
        int16_t *buf;
        if (pa_stream_peek(stream, (const void **) &buf, &length) < 0)
            return;
        if (buf)
            that->buffer->write(buf, length / sizeof(int16_t));
        pa_stream_drop(stream);
    }
  }

  std::string device;
  pa_threaded_mainloop *mainloop;
  pa_context *context;
  pa_stream *stream;
  SPtr<Buffer<int16_t>> buffer;
};
#ifndef PKGED
void PAInput::stop() {
  if (!stream) return;
  Lock lock(mainloop);
  pa_stream_disconnect(stream);
  pa_stream_unref(stream);
  stream = nullptr;
}
void PAInput::start() { // key
  Lock lock(mainloop);

  pa_sample_spec sample_spec = { };
  sample_spec.format = PA_SAMPLE_S16LE;
  sample_spec.rate = (uint32_t) config::freq_sample;
  // TODO stereo support
  sample_spec.channels = (uint8_t) (config::stereo? 2: 1);

  stream = pa_stream_new(context, "abeat input", &sample_spec, nullptr);
  if (!stream) throw std::runtime_error("Failed to create PA stream");
  pa_stream_set_state_callback(stream, stream_state_callback, this);
  pa_stream_set_read_callback(stream, stream_read_callback, this);
  // TODO custom device
  pa_buffer_attr buffer_attr = {
      (uint32_t) -1,
      (uint32_t) -1,
      (uint32_t) -1,
      (uint32_t) -1,
      (uint32_t) config::freq_sample,
  };
  if (pa_stream_connect_record(stream, device.data(), &buffer_attr, PA_STREAM_ADJUST_LATENCY) < 0)
    throw std::runtime_error("Failed to connect PA stream");
  pa_threaded_mainloop_wait(mainloop);
  if (pa_stream_get_state(stream) != PA_STREAM_READY)
    throw std::runtime_error("PA stream is not ready");
}
PAInput::PAInput(SPtr<Buffer<int16_t>> buffer): buffer(std::move(buffer)) {
  if (!(mainloop = pa_threaded_mainloop_new()))
      throw std::runtime_error("Failed to create PA mainloop");
  Lock lock(mainloop);
  context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), "abeat");
  if (!context) {
      lock.unlock();
      pa_threaded_mainloop_free(mainloop);
      throw std::runtime_error("Failed to create PA context");
  }
  auto destroy = [&]() {
      pa_context_disconnect(context);
      pa_context_unref(context);
      lock.unlock();
      pa_threaded_mainloop_free(mainloop);
  };
  pa_context_set_state_callback(context, context_state_callback, this);
  if (pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0 ||
      pa_threaded_mainloop_start(mainloop) < 0) {
      destroy();
      throw std::runtime_error("Failed to connect PA context");
  }
  pa_threaded_mainloop_wait(mainloop);
  if (pa_context_get_state(context) != PA_CONTEXT_READY) {
      pa_threaded_mainloop_stop(mainloop);
      destroy();
      throw std::runtime_error("PA context no ready");
  }
  pa_operation *operation;
  operation = pa_context_get_server_info(context, context_info_callback, this);
  while (pa_operation_get_state(operation) != PA_OPERATION_DONE)
      pa_threaded_mainloop_wait(mainloop);
  pa_operation_unref(operation);
}
#endif
} // namespace abeat