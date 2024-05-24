cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("netlink")
    add_subdirectory("rtnetlink")
    add_subdirectory("sockdiag")
    add_subdirectory("lib-mnl")
    add_subdirectory("libnl")
endif()
