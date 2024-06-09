cmake_minimum_required(VERSION 3.10)

if (WIN32)
    add_subdirectory("pil")
    add_subdirectory("get_adapters")
endif()
