cmake_minimum_required(VERSION 3.10)

project(libevpp-echo-server C CXX)

find_library(LIBEV ev REQUIRED)
set(${PROJECT_NAME}_SRC libevpp_echo_server.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" "${LIBEV}")

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" wsock32 ws2_32)
endif()

