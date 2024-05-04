cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("libevent_echo_server")
    add_subdirectory("libevpp_echo_server")
    add_subdirectory("libuv_echo_server")
    add_subdirectory("c_ares_example")
    add_subdirectory("getaddrinfo_a_resolver")
endif()
