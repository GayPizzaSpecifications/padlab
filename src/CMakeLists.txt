set(SOURCES
	maths.h
	draw.h
	draw_common.c
	$<$<NOT:$<OR:$<BOOL:${USE_OPENGL}>,$<BOOL:${USE_OPENGL_LEGACY}>>>:draw.c>
	$<$<BOOL:${USE_OPENGL}>:draw_opengl_core.c>
	$<$<BOOL:${USE_OPENGL_LEGACY}>:draw_opengl.c>
	stick.h
	stick.c
	analogue.c)

if (USE_OPENGL)
	include(GL3WHelper)
	add_gl3w(gl3w)
endif()

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET}
	$<$<PLATFORM_ID:Windows>:SDL2::SDL2main>
	SDL2::SDL2
	$<$<OR:$<BOOL:${USE_OPENGL}>,$<BOOL:${USE_OPENGL_LEGACY}>>:OpenGL::GL>
	$<$<BOOL:${USE_OPENGL}>:gl3w>
	$<$<BOOL:${GNU}>:m>)
target_compile_options(${TARGET} PRIVATE
	$<$<BOOL:${GNU}>:-Wall -Wextra -pedantic -Wno-unused-parameter>)
target_compile_definitions(${TARGET} PRIVATE
	$<$<OR:$<BOOL:${USE_OPENGL}>,$<BOOL:${USE_OPENGL_LEGACY}>>:USE_OPENGL>)
