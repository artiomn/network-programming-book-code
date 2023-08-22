cmake_minimum_required(VERSION 3.10)

add_subdirectory("select_aio_server")

if (UNIX)
    add_subdirectory("libevent_echo_server")
    add_subdirectory("libevpp_echo_server")
    add_subdirectory("libuv_echo_server")
    add_subdirectory("c_ares_example")
    add_subdirectory("asio_server")
    add_subdirectory("aio_example")
    add_subdirectory("epoll_example")
#    add_subdirectory("ioring_server")
endif()
