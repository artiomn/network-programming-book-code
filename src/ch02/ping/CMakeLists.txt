cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}ping" C CXX)

set(${PROJECT_NAME}_SRC ping.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}-from-user" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}-from-user" socket-wrapper)

add_executable("${PROJECT_NAME}-from-root" "ping-from-root.cpp")
target_link_libraries("${PROJECT_NAME}-from-root" socket-wrapper)
