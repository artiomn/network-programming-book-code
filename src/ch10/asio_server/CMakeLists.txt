cmake_minimum_required(VERSION 3.10)

project(asio-server C CXX)

find_package(Boost REQUIRED COMPONENTS system) # asio

set(${PROJECT_NAME}_SRC main.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_include_directories("${PROJECT_NAME}" PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries("${PROJECT_NAME}" PRIVATE ${Boost_LIBRARIES} pthread)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()
