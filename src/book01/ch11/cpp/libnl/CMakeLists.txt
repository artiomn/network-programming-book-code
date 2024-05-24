cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}libnl" C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(LibNL REQUIRED)
add_executable("${PROJECT_NAME}-example" main.cpp)
target_include_directories("${PROJECT_NAME}-example" PRIVATE "${LibNL_INCLUDE_DIR}")
target_link_libraries("${PROJECT_NAME}-example" "${LibNL_LIBRARIES}")
