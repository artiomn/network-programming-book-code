cmake_minimum_required(VERSION 3.10)

add_subdirectory("reuse_addr_test")
add_subdirectory("reuse_addr_experiment")
add_subdirectory("mt_server")
add_subdirectory("accept_pool")

if (UNIX)
    add_subdirectory("scatter_gatter")
    # if (LINUX)
        add_subdirectory("tcp_congestion_control")
    # endif()
endif()
