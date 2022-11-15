cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(padlab C)
set(CMAKE_C_STANDARD 99)
set(TARGET padlab)

find_package(SDL2 REQUIRED)
find_package(OpenGL)

set(SOURCES
	maths.h
	draw.h
	stick.c stick.h
	analogue.c)

if (OPENGL_FOUND)
	list(APPEND SOURCES draw_opengl.c)
else()
	list(APPEND SOURCES draw.c)
endif()

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} SDL2::SDL2 m)
if (OPENGL_FOUND)
	target_link_libraries(${TARGET} OpenGL::GL)
endif()
target_compile_options(${TARGET} PRIVATE
	-Wall -Wextra -pedantic -Wno-unused-parameter)
