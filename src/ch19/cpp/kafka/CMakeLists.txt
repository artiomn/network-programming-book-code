cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}kafka-example" C CXX)

find_package(CppKafka REQUIRED)

add_executable("${PROJECT_NAME}" "producer.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE CppKafka::cppkafka)
