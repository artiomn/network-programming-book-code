cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("select_aio_server")
endif()
