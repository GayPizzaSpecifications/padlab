include(CMakeParseArguments) # 3.4 and lower compatibility

function (_xrun_find_program OUTPUT NAME)
	find_program(XCRUN_EXECUTABLE xcrun REQUIRED)
	execute_process(COMMAND ${XCRUN_EXECUTABLE} -sdk macosx -f ${NAME} OUTPUT_VARIABLE EXECUTABLE_PATH)
	get_filename_component(EXECUTABLE_DIR ${EXECUTABLE_PATH} DIRECTORY)
	find_program(${OUTPUT} ${NAME} PATHS ${EXECUTABLE_DIR} REQUIRED)
endfunction()

function (metal_compile)
	cmake_parse_arguments(ARGS "DEBUG" "OUTPUT" "SOURCES;CFLAGS" ${ARGN})

	_xrun_find_program(METAL_EXECUTABLE metal)

	if (ARGS_DEBUG)
		_xrun_find_program(METAL_DSYMUTIL_EXECUTABLE metal-dsymutil)
		list(APPEND CFLAGS -frecord-sources)
	else()
		_xrun_find_program(METALLIB_EXECUTABLE metallib)
	endif()

	set(AIR_OBJECTS)
	foreach (SOURCE ${ARGS_SOURCES})
		if (${CMAKE_VERSION} VERSION_GREATER 3.3)
			get_filename_component(SOURCE "${SOURCE}" REALPATH)
		else()
			if (NOT IS_ABSOLUTE ${SOURCE})
				set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE})
			endif()
		endif()
		get_filename_component(BASENAME "${SOURCE}" NAME)
		set(OUTPUT_AIR ${BASENAME}.air)
		add_custom_command(
			COMMAND ${METAL_EXECUTABLE}
			ARGS ${ARGS_CFLAGS} -c ${SOURCE} -o ${OUTPUT_AIR}
			DEPENDS ${METAL_EXECUTABLE} ${SOURCE}
			OUTPUT ${OUTPUT_AIR})
		list(APPEND AIR_OBJECTS ${OUTPUT_AIR})
	endforeach()

	if (NOT ARGS_DEBUG)
	add_custom_command(
		COMMAND ${METALLIB_EXECUTABLE}
		ARGS ${LDFLAGS} ${AIR_OBJECTS} -o ${ARGS_OUTPUT}
		DEPENDS ${METALLIB_EXECUTABLE} ${AIR_OBJECTS}
		OUTPUT ${ARGS_OUTPUT})
	else()
		set(OUTPUTSYM ${ARGS_OUTPUT}sym)
		add_custom_command(
			COMMAND ${METAL_EXECUTABLE} ARGS -frecord-sources ${AIR_OBJECTS} -o ${ARGS_OUTPUT}
			COMMAND ${METAL_DSYMUTIL_EXECUTABLE} ARGS -flat -remove-source ${ARGS_OUTPUT}
			DEPENDS ${METAL_EXECUTABLE} ${METAL_DSYMUTIL_EXECUTABLE} ${AIR_OBJECTS}
			OUTPUT ${ARGS_OUTPUT} ${OUTPUTSYM})
	endif()
endfunction()
