cmake_minimum_required(VERSION 3.10)

get_filename_component(CHAPTER_NAME "${CMAKE_CURRENT_LIST_DIR}-" NAME CACHED)

if (UNIX)
    add_subdirectory("lwip_example")
    add_subdirectory("cpp-httplib-example")
    add_subdirectory("curl-example")
    add_subdirectory("restbed-example")
endif()
