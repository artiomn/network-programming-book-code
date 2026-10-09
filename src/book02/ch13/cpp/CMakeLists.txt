cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("scatter_gatter")
elseif (WIN32)
    add_subdirectory("connectx_example")
    add_subdirectory("rio_example")
endif()
