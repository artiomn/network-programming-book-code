cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("socket_pair_test")
    add_subdirectory("unix_socket_server")
endif()
