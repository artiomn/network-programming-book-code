cmake_minimum_required(VERSION 3.10)

get_filename_component(CHAPTER_NAME "${CMAKE_CURRENT_LIST_DIR}-" NAME CACHED)

add_subdirectory("reuse_addr_experiment")
add_subdirectory("mt_server")
