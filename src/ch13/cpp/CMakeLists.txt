cmake_minimum_required(VERSION 3.10)

add_subdirectory(csv_parser)
add_subdirectory(thrift)
add_subdirectory(flatbuffers)

if (UNIX)
    add_subdirectory("phistache-example")
endif()
