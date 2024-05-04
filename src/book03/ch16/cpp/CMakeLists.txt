cmake_minimum_required(VERSION 3.10)


if(NOT WIN32)
    add_subdirectory(mqtt)
    add_subdirectory(amqp)
endif()
