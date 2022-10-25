cmake_minimum_required(VERSION 3.10)

get_filename_component(CHAPTER_NAME "${CMAKE_CURRENT_LIST_DIR}-" NAME CACHED)

if (UNIX)
    add_subdirectory("if_nameindex")
    add_subdirectory("if_rename")
    add_subdirectory("sysconf")
    add_subdirectory("netlink")
    add_subdirectory("sockdiag")
endif()
