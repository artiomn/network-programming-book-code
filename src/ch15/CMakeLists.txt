cmake_minimum_required(VERSION 3.10)

get_filename_component(CHAPTER_NAME "${CMAKE_CURRENT_LIST_DIR}-" NAME CACHED)

find_package(OpenSSL)

if (OpenSSL_FOUND)
    add_subdirectory("openssl-client")
endif()

if (UNIX)
    add_subdirectory("openssl-socket-client")
    add_subdirectory("jwt-cpp-example")
endif()
