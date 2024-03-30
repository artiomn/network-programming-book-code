cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}ioring-server" C CXX)

set(${PROJECT_NAME}_SRC main.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

find_library(URING uring REQUIRED)

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" PRIVATE socket-wrapper "${URING}")
