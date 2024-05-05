cmake_minimum_required(VERSION 3.10)

if (WIN32)
    add_subdirectory("wping")
    add_subdirectory("winet")
endif()
