cmake_minimum_required(VERSION 3.10)

project(openssl-socket-client C CXX)

find_package(OpenSSL REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" pthread OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries("${PROJECT_NAME}" socket-wrapper)

