cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}stacktrace" C CXX)


set(CMAKE_CXX_STANDARD 23)
add_executable("${PROJECT_NAME}" "main.cpp")
