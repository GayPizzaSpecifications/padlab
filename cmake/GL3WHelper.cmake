find_package(Python REQUIRED COMPONENTS Interpreter)

function (add_gl3w _GL3W_TARGET)
	find_file(GL3W_GEN_EXECUTABLE gl3w_gen.py PATHS ${CMAKE_SOURCE_DIR}/tools)

	set(GL3W_ROOT ${CMAKE_CURRENT_BINARY_DIR}/gl3w)
	set(GL3W_INCLUDE_DIR ${GL3W_ROOT}/include)
	set(GL3W_SOURCES
		${GL3W_INCLUDE_DIR}/GL/gl3w.h
		${GL3W_INCLUDE_DIR}/GL/glcorearb.h
		${GL3W_INCLUDE_DIR}/KHR/khrplatform.h
		${GL3W_ROOT}/src/gl3w.c)

	add_custom_command(
		COMMAND Python::Interpreter
		ARGS ${GL3W_GEN_EXECUTABLE} --root=${GL3W_ROOT}
		DEPENDS Python::Interpreter ${GL3W_GEN_EXECUTABLE}
		OUTPUT ${GL3W_SOURCES})

	add_library(${_GL3W_TARGET} ${GL3W_SOURCES})
	target_include_directories(${_GL3W_TARGET} PUBLIC ${GL3W_INCLUDE_DIR})
endfunction()
