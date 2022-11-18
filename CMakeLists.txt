cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(padlab C)
set(TARGET padlab)

option(USE_OPENGL "Use legacy OpenGL for drawing" ON)

set(CMAKE_C_STANDARD 99)

find_package(SDL2 REQUIRED)
if (USE_OPENGL)
	find_package(OpenGL REQUIRED)
endif()

set(SOURCES
	src/maths.h
	src/draw.h     src/$<IF:$<BOOL:${USE_OPENGL}>,draw_opengl.c,draw.c>
	src/stick.c    src/stick.h
	src/analogue.c)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET}
	SDL2::SDL2 $<$<BOOL:${USE_OPENGL}>:OpenGL::GL> m)
target_compile_options(${TARGET} PRIVATE
	-Wall -Wextra -pedantic -Wno-unused-parameter)
target_compile_definitions(${TARGET} PRIVATE
	$<$<BOOL:${USE_OPENGL}>:USE_OPENGL>)
