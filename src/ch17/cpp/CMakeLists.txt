cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("lwip_example")
    add_subdirectory("cpp-httplib-example")
    add_subdirectory("curl-example")
    add_subdirectory("restbed-example")
endif()
