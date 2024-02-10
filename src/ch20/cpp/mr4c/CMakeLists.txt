cmake_minimum_required(VERSION 3.22)
project("${CHAPTER_NAME}mr4c-example")

set(MR4C_PATH "/usr/local/mr4c/native")

find_library(MR4C NAMES mr4c HINTS "${MR4C_PATH}/dist" REQUIRED)

add_library("${PROJECT_NAME}" SHARED "main.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE "${MR4C}")
target_include_directories("${PROJECT_NAME}" PRIVATE "${MR4C_PATH}/include")
