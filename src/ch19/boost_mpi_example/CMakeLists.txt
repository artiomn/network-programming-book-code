cmake_minimum_required(VERSION 3.16)
project(boost-mpi-example LANGUAGES CXX)

find_package(Boost COMPONENTS mpi serialization REQUIRED)

add_executable("${PROJECT_NAME}-messaging" messaging.cpp)
add_executable("${PROJECT_NAME}-reduce" reduce.cpp)

target_link_libraries("${PROJECT_NAME}-messaging" PRIVATE Boost::mpi)
target_link_libraries("${PROJECT_NAME}-reduce" PRIVATE Boost::mpi)
