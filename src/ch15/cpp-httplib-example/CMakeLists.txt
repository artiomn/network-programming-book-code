cmake_minimum_required(VERSION 3.10)

project(cpp-httplib-example C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

add_executable("${PROJECT_NAME}-server" "server.cpp")
target_link_libraries("${PROJECT_NAME}-server" pthread)
add_executable("${PROJECT_NAME}-client" "client.cpp")
target_link_libraries("${PROJECT_NAME}-client" pthread OpenSSL::SSL OpenSSL::Crypto)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()

