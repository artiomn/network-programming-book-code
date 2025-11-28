cmake_minimum_required(VERSION 3.10)

add_subdirectory("reuse_addr_test")
add_subdirectory("reuse_addr_experiment")

if (UNIX)
    # if (LINUX)
        add_subdirectory("tcp_congestion_control")
    # endif()
endif()
