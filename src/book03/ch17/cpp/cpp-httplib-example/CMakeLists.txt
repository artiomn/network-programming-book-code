cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}cpp-httplib-example" C CXX)

find_package(OpenSSL REQUIRED)

add_executable("${PROJECT_NAME}-server" "server.cpp")
add_executable("${PROJECT_NAME}-client" "client.cpp")
target_link_libraries("${PROJECT_NAME}-client" OpenSSL::SSL OpenSSL::Crypto)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()
