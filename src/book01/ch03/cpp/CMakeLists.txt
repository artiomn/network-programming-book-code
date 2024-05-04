cmake_minimum_required(VERSION 3.10)

add_subdirectory("tcp_client")
add_subdirectory("socket_address")

if (UNIX)
    add_subdirectory("oob_example")
endif()