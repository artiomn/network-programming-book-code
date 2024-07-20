cmake_minimum_required(VERSION 3.10)


if (UNIX)
    add_subdirectory("linux_call_intercepter")
endif()
