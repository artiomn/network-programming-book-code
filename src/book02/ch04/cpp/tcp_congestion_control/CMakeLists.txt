cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}congestion-control" C CXX)

set(${PROJECT_NAME}_SRC main.cpp)

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" PRIVATE socket-wrapper)
