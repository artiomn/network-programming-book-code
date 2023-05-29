cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}simple-blockchain" C CXX)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" OpenSSL::SSL OpenSSL::Crypto)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()
