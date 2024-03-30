cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}seastar-example" C CXX)

find_package (Seastar REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE Seastar::seastar)
