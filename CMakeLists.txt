cmake_minimum_required(VERSION 3.16)
project(abeat)

find_package(Threads REQUIRED)
find_package(PulseAudio REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW3 REQUIRED fftw3f libfftw3f)

set(CMAKE_CXX_STANDARD 17)

add_definitions(${OPENGL_DEFINITIONS})
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/gl/include
	${PULSEAUDIO_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIRS})

set(PULSE_LIBS ${PULSE_LIBS} "pulse")
set(SHARED_SRC buffer.cpp FFT.cpp sndin_pulseaudio.cpp gl_render.cpp)
set(SHARED_INLINE types.hpp gl_my.hpp)
set(SHADER_SRC res/bar.shader res/barg.shader)
set(S ${CMAKE_CURRENT_SOURCE_DIR}/)

add_custom_command(OUTPUT ${S}pkged/FFT.h
	PRE_BUILD COMMAND cd ${S}&&mkdir -p pkged &&python mkhpp.py ${SHARED_SRC} ${SHARED_INLINE} 1>/dev/null
	DEPENDS ${SHARED_SRC})
add_custom_command(OUTPUT ${S}gl/src/glad.c
	PRE_BUILD COMMAND cp ${S}gl.xml.gz . &&gzip -d -fk gl.xml.gz
	&&python -m glad --generator=c --profile=compatibility --spec=gl --reproducible --api="gl=4.0" --out-path ${S}gl
	DEPENDS gl.xml.gz)
add_custom_command(OUTPUT ${S}pkged/gl_shaders.hpp
	PRE_BUILD COMMAND cd ${S}&&mkdir -p pkged &&python mk_glsl_hpp.py ${SHADER_SRC} >pkged/gl_shaders.hpp
	DEPENDS ${SHADER_SRC}
)

add_executable(abeat
	main.cpp ${SHARED_SRC}
    pkged/FFT.h pkged/gl_shaders.hpp ${S}gl/src/glad.c)

target_link_libraries(abeat
	${CMAKE_THREAD_LIBS_INIT}
	${PULSE_LIBS}
	${OPENGL_LIBRARIES}
	${FFTW3_LIBRARIES}
	glfw dl)
