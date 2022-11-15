cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(padlab C)
set(TARGET padlab)

set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED)
find_package(OpenGL)

set(SOURCES
	maths.h
	draw.h  $<IF:$<BOOL:${OPENGL_FOUND}>,draw_opengl.c,draw.c>
	stick.c stick.h
	analogue.c)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET}
	SDL2::SDL2 $<$<BOOL:${OPENGL_FOUND}>:OpenGL::GL> m)
target_compile_options(${TARGET} PRIVATE
	-Wall -Wextra -pedantic -Wno-unused-parameter)
