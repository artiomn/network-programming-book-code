cmake_minimum_required(VERSION 3.10)


if(NOT WIN32)
    add_subdirectory(zeromq)
    add_subdirectory(kafka)
    add_subdirectory(nats)
    add_subdirectory(mqtt)
endif()
