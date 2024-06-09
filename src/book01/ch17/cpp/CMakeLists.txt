cmake_minimum_required(VERSION 3.10)

if (WIN32)
    add_subdirectory("mailslot")
    add_subdirectory("named_pipe")
endif()
