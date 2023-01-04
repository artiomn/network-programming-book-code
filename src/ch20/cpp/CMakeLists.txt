cmake_minimum_required(VERSION 3.10)

get_filename_component(CHAPTER_NAME "${CMAKE_CURRENT_LIST_DIR}-" NAME CACHED)

if (UNIX)
    add_subdirectory("qt-wordcount")
    add_subdirectory("libtorrent")
    add_subdirectory("openmpi_example")
    add_subdirectory("boost_mpi_example")
endif()
