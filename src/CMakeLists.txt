add_subdirectory(modules)
add_subdirectory(client)
if (SERVER)
	add_subdirectory(server)
endif()
add_subdirectory(tools)
if (VISUALTESTS)
	add_subdirectory(tests)
endif()
