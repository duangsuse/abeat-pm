#include "pkged/types.hpp"
#include "config.hpp"
#include "pkged/buffer.hpp"

#include <pulse/pulseaudio.h>
namespace abeat {
#ifndef PKGED
DEFINE_LOCK(PA, pa_threaded_mainloop)
#endif
struct PAInput {
public:
  explicit PAInput(SPtr<Buffer<spnum>> buffer);
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
  void kill() { pa_threaded_mainloop_signal(this->mainloop, 0); }
#define PA_CALLBACK(name, op, ...) static void name##_callback(__VA_ARGS__, void *userdata) \
  { auto *self = (PAInput*)userdata; op }
  PA_CALLBACK(context_info, {
    self->device = info->default_sink_name;
    self->device += ".monitor";
    self->kill();
  }, pa_context *context, const pa_server_info *info)
  PA_CALLBACK(stream_read, {
    spnum *buf;
    while (pa_stream_readable_size(stream) > 0) {
      if (pa_stream_peek(stream, (const void **) &buf, &length) < 0) break; // read: peek&drop
      if (buf) self->buffer->write(buf, length / sizeof(spnum));
      pa_stream_drop(stream);
    }
  }, pa_stream *stream, size_t length)

  PA_CALLBACK(context_state, switch (pa_context_get_state(context)) {
    case PA_CONTEXT_READY:
    case PA_CONTEXT_TERMINATED:
    case PA_CONTEXT_FAILED: self->kill(); break;
    default: break;
  }, pa_context *context)
  PA_CALLBACK(stream_state, switch (pa_stream_get_state(stream)) {
    case PA_STREAM_READY:
    case PA_STREAM_TERMINATED:
    case PA_STREAM_FAILED: self->kill(); break;
    default: break;
  }, pa_stream *stream)

  pa_context *context; std::string device;
  pa_stream *stream;
  pa_threaded_mainloop *mainloop;
  SPtr<Buffer<spnum>> buffer;
};
#ifndef PKGED
void PAInput::stop() {
  if (!stream) return;
  PALock lock(mainloop);
  pa_stream_disconnect(stream);
  pa_stream_unref(stream);
  stream = nullptr;
}
void PAInput::start() { // key
  PALock lock(mainloop);
#define fails(verb) ("Failed to " verb " PA stream")
  pa_sample_spec sample_spec = { .format = PA_SAMPLE_S16LE, .rate = config::freq_sample, .channels = config::channels };
  stream = notZero(pa_stream_new(context, "abeat input", &sample_spec, nullptr), fails("create"));
  pa_buffer_attr buffer_attr = {
      (uint32_t) -1,
      (uint32_t) -1,
      (uint32_t) -1,
      (uint32_t) -1,
      (uint32_t) config::freq_sample,
  };
  pa_stream_set_state_callback(stream, stream_state_callback, this); // order: steam state/read, context state/info
  pa_stream_set_read_callback(stream, stream_read_callback, this);
  notNeg(pa_stream_connect_record(stream, device.data(), &buffer_attr, PA_STREAM_ADJUST_LATENCY), fails("connect")); // TODO custom device string
  pa_threaded_mainloop_wait(mainloop);
  notNeq(PA_STREAM_READY, pa_stream_get_state(stream), "PA stream is not ready");
}
#undef fails
#define fails(verb) ("Failed to " verb " PA context")
PAInput::PAInput(SPtr<Buffer<spnum>> buffer): buffer(std::move(buffer)) {
  notZero((mainloop = pa_threaded_mainloop_new()), fails("create"));
  PALock lock(mainloop);
  context = pa_context_new(pa_threaded_mainloop_get_api(mainloop), "abeat");
  if (context == nullptr) {
      lock.unlock();
      pa_threaded_mainloop_free(mainloop);
      throw std::runtime_error(fails("create"));
  }
  auto destroy = [&]() {
      pa_context_disconnect(context);
      pa_context_unref(context);
      lock.unlock();
      pa_threaded_mainloop_free(mainloop);
  };
  pa_context_set_state_callback(context, context_state_callback, this);
  try {
    notNeg(pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr), fails("connect"));
    notNeg(pa_threaded_mainloop_start(mainloop), fails("connect"));
    pa_threaded_mainloop_wait(mainloop);
    notNeq(PA_CONTEXT_READY, pa_context_get_state(context), "PA context no ready");
  }  catch (std::runtime_error ex) { pa_threaded_mainloop_stop(mainloop); destroy(); throw std::move(ex); }

  pa_operation *operation = pa_context_get_server_info(context, context_info_callback, this); // query info
  while (pa_operation_get_state(operation) != PA_OPERATION_DONE) pa_threaded_mainloop_wait(mainloop);
  pa_operation_unref(operation);
}
#undef fail
#endif
} // namespace abeat
