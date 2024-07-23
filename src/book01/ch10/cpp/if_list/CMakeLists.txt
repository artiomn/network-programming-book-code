cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}if_list" C CXX)

add_executable("${PROJECT_NAME}" main.cpp)
target_link_libraries("${PROJECT_NAME}" socket-wrapper)
