cmake_minimum_required(VERSION 3.10)

if (WIN32)
    add_subdirectory("service_base")
    add_subdirectory("netbios")
endif()
