cmake_minimum_required(VERSION 3.10)

get_filename_component(CHAPTER_NAME "${CMAKE_CURRENT_LIST_DIR}-" NAME CACHED)

add_executable("${CHAPTER_NAME}test_byte_order" test_byte_order.cpp)

add_subdirectory(resolve)

