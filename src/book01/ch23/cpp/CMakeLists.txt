cmake_minimum_required(VERSION 3.10)


if (UNIX)
    add_subdirectory("tun_example")
    add_subdirectory("zc_example")
endif()
