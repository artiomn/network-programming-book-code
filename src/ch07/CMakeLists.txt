cmake_minimum_required(VERSION 3.10)

get_filename_component(CHAPTER_NAME "${CMAKE_CURRENT_LIST_DIR}-" NAME CACHED)

if (UNIX)
    add_subdirectory("linux_call_intercepter")
    add_subdirectory("signal_handling")
endif()

add_subdirectory("incorrect_udp_client")
