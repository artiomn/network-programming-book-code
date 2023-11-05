cmake_minimum_required(VERSION 3.10)

add_subdirectory(csv_parser)
add_subdirectory(thrift)

find_package(flatbuffers)

if (flatbuffers_FOUND)
    add_subdirectory(flatbuffers)
else()
    message(WARNING "Please install Flatbuffers!")
endif()

if (UNIX)
    add_subdirectory("grpc-example")
    add_subdirectory("phistache-example")
endif()
