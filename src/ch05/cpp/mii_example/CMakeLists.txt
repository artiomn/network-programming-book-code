cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}mii-example" C CXX)

set(${PROJECT_NAME}_SRC mii-example.cpp)

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
