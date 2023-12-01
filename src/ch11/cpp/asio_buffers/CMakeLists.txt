cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}asio-buffers" C CXX)

find_package(Boost REQUIRED COMPONENTS system) # asio

set(${PROJECT_NAME}_SRC main.cpp)

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_include_directories("${PROJECT_NAME}" PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries("${PROJECT_NAME}" PRIVATE ${Boost_LIBRARIES})
