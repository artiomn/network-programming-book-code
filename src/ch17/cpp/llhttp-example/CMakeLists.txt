cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}llhttp-example" C CXX)

find_package(llhttp REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE llhttp::llhttp_shared)
