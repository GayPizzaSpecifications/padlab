include(CMakeParseArguments) # 3.4 and lower compatibility
find_package(Python REQUIRED COMPONENTS Interpreter)

function (bin2h_compile)
	set(oneValueArgs OUTPUT)
	set(multiValueArgs BIN TXT)
	cmake_parse_arguments(ARGS "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	find_file(BIN2H_EXECUTABLE bin2h.py PATHS ${CMAKE_SOURCE_DIR}/tools)
	set(DEPENDS)
	set(COMMAND ${BIN2H_EXECUTABLE})

	foreach (BIN ${ARGS_BIN})
		set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${BIN})
		list(APPEND DEPENDS ${SOURCE})
		list(APPEND COMMAND "-b" "${SOURCE}")
	endforeach()

	foreach (TXT ${ARGS_TXT})
		set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${TXT})
		list(APPEND DEPENDS ${SOURCE})
		list(APPEND COMMAND "-t" "${SOURCE}")
	endforeach()

	list(APPEND COMMAND ${ARGS_OUTPUT})
	add_custom_command(COMMAND Python::Interpreter ARGS ${COMMAND}
		DEPENDS Python::Interpreter ${BIN2H_EXECUTABLE} ${DEPENDS}
		OUTPUT ${ARGS_OUTPUT})
endfunction()
