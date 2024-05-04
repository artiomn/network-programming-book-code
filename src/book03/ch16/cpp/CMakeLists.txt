cmake_minimum_required(VERSION 3.10)

find_package(OpenSSL)

if (OpenSSL_FOUND)
    add_subdirectory("openssl-client")
    add_subdirectory("openssl-socket-server")
    add_subdirectory("openssl-socket-client")
endif()

if (UNIX)
    add_subdirectory("jwt-cpp-example")
endif()
