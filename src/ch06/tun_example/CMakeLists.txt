cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}tun-example" C CXX)

set(${PROJECT_NAME}_SRC main.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
