cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(padlab C)
set(TARGET padlab)

option(USE_OPENGL "Use legacy OpenGL for drawing" ON)

set(CMAKE_C_STANDARD 99)
set(GNU_COMPILERS GNU Clang AppleClang)
if (CMAKE_C_COMPILER_ID IN_LIST GNU_COMPILERS)
	set(GNU 1)
elseif (MSVC)
	string(REPLACE "/W3" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endif()

find_package(SDL2 REQUIRED)
if (USE_OPENGL)
	if (NOT DEFINED OpenGL_GL_PREFERENCE)
		set(OpenGL_GL_PREFERENCE "GLVND")
	endif()
	find_package(OpenGL REQUIRED)
endif()

set(SOURCES
	src/maths.h
	src/draw.h     src/$<IF:$<BOOL:${USE_OPENGL}>,draw_opengl.c,draw.c>
	src/stick.c    src/stick.h
	src/analogue.c)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET}
	$<$<PLATFORM_ID:Windows>:SDL2::SDL2main>
	SDL2::SDL2
	$<$<BOOL:${USE_OPENGL}>:OpenGL::GL>
	$<$<BOOL:${GNU}>:m>)
target_compile_options(${TARGET} PRIVATE
	$<$<BOOL:${GNU}>:-Wall -Wextra -pedantic -Wno-unused-parameter>)
target_compile_definitions(${TARGET} PRIVATE
	$<$<BOOL:${USE_OPENGL}>:USE_OPENGL>)
