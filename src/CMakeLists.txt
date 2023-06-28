set(SOURCES_COMMON
	maths.h
	draw.h
	draw_common.c
	stick.h
	stick.c
	analogue.c)
set(SOURCES_SDL_RENDERER draw.c)
set(SOURCES_METAL metal/draw_metal.m metal/metal_shader_types.h)
set(SOURCES_OPENGL glcore/draw_opengl_core.c)
set(SOURCES_OPENGL_LEGACY gl/draw_opengl.c)

function (common_setup _TARGET)
	target_link_libraries(${_TARGET}
		$<$<PLATFORM_ID:Windows>:SDL2::SDL2main>
		SDL2::SDL2
		$<$<BOOL:${GNU}>:m>)
	target_compile_options(${_TARGET} PRIVATE
		$<$<BOOL:${GNU}>:-Wall -Wextra -pedantic -Wno-unused-parameter>)
endfunction()

add_executable(${TARGET} ${SOURCES_COMMON} ${SOURCES_SDL_RENDERER})
common_setup(${TARGET})

if (BUILD_METAL OR BUILD_OPENGL)
	include(BinHelper)
endif()

if (BUILD_METAL)
	enable_language(OBJC)
	find_library(METAL Metal REQUIRED)
	find_library(FOUNDATION Foundation REQUIRED)
	find_library(QUARTZCORE QuartzCore REQUIRED)
	include(MetalHelper)
	metal_compile(OUTPUT shader.metallib SOURCES metal/shader.metal)
	bin2h_compile(OUTPUT metalShader.h BIN ${CMAKE_CURRENT_BINARY_DIR}/shader.metallib)
	add_executable(${TARGET}_metal ${SOURCES_COMMON} ${SOURCES_METAL} ${CMAKE_CURRENT_BINARY_DIR}/metalShader.h)
	target_include_directories(${TARGET}_metal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(${TARGET}_metal ${METAL} ${QUARTZCORE} ${FOUNDATION})
	target_compile_definitions(${TARGET}_metal PRIVATE USE_METAL)
	common_setup(${TARGET}_metal)
endif()

if (BUILD_OPENGL)
	include(GL3WHelper)
	add_gl3w(gl3w)
	bin2h_compile(OUTPUT glslShaders.h TXT glcore/vert.glsl glcore/geom.glsl glcore/frag.glsl)
	add_executable(${TARGET}_glcore ${SOURCES_COMMON} ${SOURCES_OPENGL} ${CMAKE_CURRENT_BINARY_DIR}/glslShaders.h)
	target_include_directories(${TARGET}_glcore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(${TARGET}_glcore OpenGL::GL gl3w)
	target_compile_definitions(${TARGET}_glcore PRIVATE USE_OPENGL)
	common_setup(${TARGET}_glcore)
endif()

if (BUILD_OPENGL_LEGACY)
	add_executable(${TARGET}_gl ${SOURCES_COMMON} ${SOURCES_OPENGL_LEGACY})
	target_include_directories(${TARGET}_gl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
	target_link_libraries(${TARGET}_gl OpenGL::GL)
	target_compile_definitions(${TARGET}_gl PRIVATE USE_OPENGL)
	common_setup(${TARGET}_gl)
endif()
