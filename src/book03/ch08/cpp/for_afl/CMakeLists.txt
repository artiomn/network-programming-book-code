cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}fuzzer-example" C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if (MSVC)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(../../../socket_wrapper socket_wrapper)
add_subdirectory(../../../ch08/cpp/incorrect_udp_client udp_client)
add_subdirectory(../../../ch03/cpp/tcp_client tcp_client)
