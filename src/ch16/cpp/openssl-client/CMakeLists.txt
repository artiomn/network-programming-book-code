cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}openssl-client" C CXX)

find_package(OpenSSL REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE OpenSSL::SSL OpenSSL::Crypto)

if(WIN32)
    target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
else()
    target_link_libraries("${PROJECT_NAME}" PRIVATE pthread)
endif()
