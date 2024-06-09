cmake_minimum_required(VERSION 3.10)

if (WIN32)
    add_subdirectory("notify")
    add_subdirectory("wping")
endif()
