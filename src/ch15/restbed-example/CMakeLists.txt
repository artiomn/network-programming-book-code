cmake_minimum_required(VERSION 3.10)

project(restbed-example C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_library(RESTBED restbed REQUIRED PATHS "/usr/local/library")
find_package(OpenSSL REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE "${RESTBED}" OpenSSL::Crypto OpenSSL::SSL)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()

