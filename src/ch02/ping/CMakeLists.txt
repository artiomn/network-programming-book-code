cmake_minimum_required(VERSION 3.10)

project(ping C CXX)

set(${PROJECT_NAME}_SRC ping.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" socket-wrapper)

add_executable("${PROJECT_NAME}-user-rights" "ping-user-rights.cpp")
target_link_libraries("${PROJECT_NAME}-user-rights" socket-wrapper)
