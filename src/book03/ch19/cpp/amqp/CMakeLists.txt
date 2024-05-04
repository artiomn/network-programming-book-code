cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}amqp-example" C CXX)

find_library(PROTON qpid-proton-cpp REQUIRED)

add_executable("${PROJECT_NAME}" "proton-example.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE "${PROTON}")
