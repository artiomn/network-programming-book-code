cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("qt-wordcount")
    add_subdirectory("openmpi_example")
    add_subdirectory("boost_mpi_example")
    add_subdirectory("mr4c")
endif()
