cmake_minimum_required(VERSION 3.10)

# add_subdirectory("stacktrace")

if (UNIX)
    add_subdirectory("signal_handling")
    add_subdirectory("incorrect_udp_client")
endif()
