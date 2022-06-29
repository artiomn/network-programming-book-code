cmake_minimum_required(VERSION 3.10)

get_filename_component(CHAPTER_NAME "${CMAKE_CURRENT_LIST_DIR}-" NAME CACHED)

add_subdirectory("oob_example")
add_subdirectory("tcp_client")

