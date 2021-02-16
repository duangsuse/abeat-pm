# Abeat with PM

OpenGL/PulseAudio visualizer, with project management scripts added; currently only mono channel is supported.

```bash
# build project, may -DCMAKE_BUILD_TYPE=Release
cmake -B build . &&cd build &&make ; cd -
# clean
rm -rf gl/ pkged/ build/
```

## Structure

+ [buffer](buffer.cpp)
+ [fourier transform](FFT.cpp)
+ [pulseaudio input](sndin_pulseaudio.cpp)
+ [render](gl_render.cpp)
+ [sharder code](gl_sharders.cpp): a RK4 interpolation "gravity", bars vert/frag/geom
+ [main dt deltaTime loop](main.cpp)

Used tool: [mkhpp.py](mkhpp.py)

## Dependencies

```bash
pip install glad --user
apt install libpulse-dev libfftw3-dev libgl-dev libglfw3-dev
pacman -S fftw glfw-x11
```
