cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}ethtool" C CXX)

set(${PROJECT_NAME}_SRC main.cpp)

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
