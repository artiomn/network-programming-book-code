cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("epoll_example")
    add_subdirectory("aio_example")
    add_subdirectory("ioring_server")
endif()
