cmake_minimum_required(VERSION 3.10)

project(libuv-echo-server C CXX)

find_library(LIBUV uv REQUIRED)
set(${PROJECT_NAME}_SRC libuv_echo_server.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" "${LIBUV}")

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" wsock32 ws2_32)
endif()

