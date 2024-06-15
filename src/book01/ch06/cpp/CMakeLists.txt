cmake_minimum_required(VERSION 3.10)


if (UNIX)
    add_subdirectory("oob_example")
	add_subdirectory(namespaces)
endif()
