cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("netlink")
    add_subdirectory("sockdiag")
endif()

if (APPLE)
    add_subdirectory(mac_if_events)
endif()
