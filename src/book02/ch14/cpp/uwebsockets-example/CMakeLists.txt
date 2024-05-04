cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}uwebsockets-example" C CXX)

set(${PROJECT_NAME}_SRC main.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

find_package(ZLIB REQUIRED)
find_library(LIB_UV NAMES uv libuv REQUIRED)
find_library(LIB_USOCKETS NAMES usockets uSockets REQUIRED HINTS /usr/local/lib)

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
set_property(TARGET "${PROJECT_NAME}" PROPERTY CXX_STANDARD 17)
set_property(TARGET "${PROJECT_NAME}" PROPERTY POSITION_INDEPENDENT_CODE TRUE)

target_link_libraries("${PROJECT_NAME}" PRIVATE "${LIB_USOCKETS}" ZLIB::ZLIB "${LIB_UV}")
