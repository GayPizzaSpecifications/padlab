set(SOURCES
	maths.h
	draw.h
	draw_common.c
	$<$<NOT:$<OR:$<STREQUAL:${GFX_BACKEND},OPENGL>,$<STREQUAL:${GFX_BACKEND},OPENGL_LEGACY>,$<STREQUAL:${GFX_BACKEND},METAL>>>:draw.c>
	$<$<STREQUAL:${GFX_BACKEND},METAL>:draw_metal.m metal_shader_types.h>
	$<$<STREQUAL:${GFX_BACKEND},OPENGL>:draw_opengl_core.c>
	$<$<STREQUAL:${GFX_BACKEND},OPENGL_LEGACY>:draw_opengl.c>
	stick.h
	stick.c
	analogue.c)

if (GFX_BACKEND STREQUAL "METAL")
	find_library(METAL Metal REQUIRED)
	find_library(FOUNDATION Foundation REQUIRED)
	find_library(QUARTZCORE QuartzCore REQUIRED)
	include(MetalHelper)
	metal_compile(OUTPUT shader.metallib SOURCES shader.metal)
	include(BinHelper)
	bin2h_compile(OUTPUT metalShader.h BIN ${CMAKE_CURRENT_BINARY_DIR}/shader.metallib)
	list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/metalShader.h)
elseif (GFX_BACKEND STREQUAL "OPENGL")
	include(GL3WHelper)
	add_gl3w(gl3w)
	include(BinHelper)
	bin2h_compile(OUTPUT glslShaders.h TXT vert.glsl geom.glsl frag.glsl)
	list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/glslShaders.h)
endif()

add_executable(${TARGET} ${SOURCES})
target_include_directories(${TARGET} PRIVATE
	$<$<OR:$<STREQUAL:${GFX_BACKEND},OPENGL>,$<STREQUAL:${GFX_BACKEND},METAL>>:${CMAKE_CURRENT_BINARY_DIR}>)
target_link_libraries(${TARGET}
	$<$<PLATFORM_ID:Windows>:SDL2::SDL2main>
	SDL2::SDL2
	$<$<STREQUAL:${GFX_BACKEND},METAL>:${METAL} ${QUARTZCORE} ${FOUNDATION}>
	$<$<OR:$<STREQUAL:${GFX_BACKEND},OPENGL>,$<STREQUAL:${GFX_BACKEND},OPENGL_LEGACY>>:OpenGL::GL>
	$<$<STREQUAL:${GFX_BACKEND},OPENGL>:gl3w>
	$<$<BOOL:${GNU}>:m>)
target_compile_options(${TARGET} PRIVATE
	$<$<BOOL:${GNU}>:-Wall -Wextra -pedantic -Wno-unused-parameter>)
target_compile_definitions(${TARGET} PRIVATE
	$<$<STREQUAL:${GFX_BACKEND},METAL>:USE_METAL>
	$<$<OR:$<STREQUAL:${GFX_BACKEND},OPENGL>,$<STREQUAL:${GFX_BACKEND},OPENGL_LEGACY>>:USE_OPENGL>)
