cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}zmq-example" C CXX)

find_package(cppzmq REQUIRED)

add_executable("${PROJECT_NAME}-push-pull" "push-pull.cpp")
target_link_libraries("${PROJECT_NAME}-push-pull" PRIVATE cppzmq)
