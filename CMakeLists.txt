cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(padlab C)
set(CMAKE_C_STANDARD 11)
set(TARGET padlab)
set(SOURCES
	maths.c maths.h
	analogue.c)

find_package(SDL2 REQUIRED)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} SDL2::SDL2 m)
target_compile_options(${TARGET} PRIVATE
	-Wall -Wextra -pedantic -Wno-unused-parameter)
