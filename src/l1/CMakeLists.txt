cmake_minimum_required(VERSION 3.10)

project(cpp-network-lesson-1 C CXX)

add_executable(test_byte_order test_byte_order.cpp)

add_subdirectory(resolve)
add_subdirectory(udp_server)
add_subdirectory(ping)

