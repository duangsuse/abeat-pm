# Abeat with PM

OpenGL/PulseAudio visualizer, with project management scripts added; currently only mono channel is supported.

```bash
# build project, may -DCMAKE_BUILD_TYPE=Release
cmake -B build . &&cd build &&make ; cd -
# clean
rm -rf gl/ pkged/ build/
```

## Dependencies

FFT(fast fourier transf.) is used for extracting freq dB histogram.

OGL framework: glad+glfw

```bash
pip install glad --user
apt install libpulse-dev libfftw3-dev libgl-dev libglfw3-dev
pacman -S fftw glfw-x11
```

## Structure

+ [buffer](buffer.cpp) ptr slice buffer with `checkUnmodify()` and mutex
+ [fourier transform](FFT.cpp) `fftwf` + Blackman Window multiplier
+ [pulseaudio input](sndin_pulseaudio.cpp) Register context/stream state and (PA server)info/read callbacks with PA mainloop&lock, provide audio sample-point buffer
+ [render](gl_render.cpp)
+ [sharder code](gl_sharders.hpp): a RK4 interpolation "gravity", bars vert/frag/geom
+ [main dt deltaTime loop](main.cpp)

Used tool: [mkhpp.py](mkhpp.py)

## Technical

This program uses various C++17 feature like `std::chrono`, `constexpr`.

`mkhpp` tool enables decl/defn reuse by outputting `{}`-removed `pkged/x.hpp` preprocessed with `cpp -DPKGED`, unless inlined members with `{1;` prefix

Main dataflow:

+ `main()` init `buffer_size`+`PAInput`+`FFT`, _startGL_, start capture & deltaTime [loop](main.cpp#L68)
+ `PAInput::stream_read_callback` peek&write&drop to `buffer` [here](sndin_pulseaudio.cpp#32)
+ `FFT::calculate(buf)` to `output[i]` with `prepare_window(window_size)`, _fftw_
+ `Render::render(float dt)` of `render.data[i] = dB;` [here](gl_render.cpp#L45), bind `fft_buf` in [ctor](gl_render.cpp#L82) 
+ `p_bar.use()` sharder program with (rect,`l,r -1/y,0,1`layout,RGBA) `link(vertex,geometry,fragment)` `u_size`, `u_half=1/usize`; and gravity `u_dt` [here](gl_sharders.hpp)

where

+ _startGL_ windowHint GLFW_OPENGL_CORE_PROFILE, `glfwCreateWindow(1000,350, "abeat")`&glContext&onresize&gladLoad
+ _fftw_ `fftwf_execute(fftwf_plan_dft_r2c_1d(window_size, input, output, FFTW_ESTIMATE))` in `fftwf_alloc_real` out `_complex`
