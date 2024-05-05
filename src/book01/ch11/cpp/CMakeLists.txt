cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("netlink")
    add_subdirectory("rtnetlink")
    add_subdirectory("sockdiag")

endif()
