cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}oob-example" C CXX)

find_library(LIBEV ev REQUIRED)
set(${PROJECT_NAME}_SRC oob_server.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" socket-wrapper)
