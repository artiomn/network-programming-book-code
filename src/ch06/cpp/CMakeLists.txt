cmake_minimum_required(VERSION 3.10)

if (WIN32)
    add_subdirectory("pil")
    add_subdirectory("wping")
    add_subdirectory("udp_exchange_client")
    add_subdirectory("udp_exchange_server")
    add_subdirectory("simple_slient")
endif()
