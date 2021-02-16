# Abeat with PM

OpenGL/PulseAudio visualizer, with project management scripts added

```bash
# build project, may -DCMAKE_BUILD_TYPE=Release
cmake -B build . &&cd build &&make ; cd -
# clean
rm -rf gl/ pkged/ build/
```

## Dependencies

```bash
apt install libpulse-dev libfftw3-dev libgl-dev libglfw3-dev
pacman -S fftw glfw-x11
```
