cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}lib-mnl" C CXX)

find_library(LIBMNL mnl REQUIRED)
add_executable("${PROJECT_NAME}-example" main.cpp)
target_link_libraries("${PROJECT_NAME}-example" "${LIBMNL}")
