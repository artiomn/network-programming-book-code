cmake_minimum_required(VERSION 3.10)

project(jwt-cpp-example C CXX)

find_package(jwt-cpp CONFIG REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" jwt-cpp::jwt-cpp)
