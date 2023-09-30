cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}getaddrinfo_a-resolver" C CXX)

set(${PROJECT_NAME}_SRC main.cpp)

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")

find_library(LIB_ANL anl REQUIRED)

target_link_libraries("${PROJECT_NAME}" PRIVATE "${LIB_ANL}")
