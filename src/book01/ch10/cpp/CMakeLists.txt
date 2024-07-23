cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("if_list")
    add_subdirectory("if_nameindex")
    add_subdirectory("if_rename")
    add_subdirectory("ethtool")
    add_subdirectory("mii_example")
endif()
