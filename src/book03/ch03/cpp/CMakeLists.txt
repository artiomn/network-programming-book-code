cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("grpc-example")
    add_subdirectory("phistache-example")
endif()
