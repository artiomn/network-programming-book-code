cmake_minimum_required(VERSION 3.10)

add_subdirectory(udp_server)
add_subdirectory(ping)

if (UNIX)
    add_subdirectory("socket_pair_test")
    add_subdirectory("unix_socket_server")
endif()
