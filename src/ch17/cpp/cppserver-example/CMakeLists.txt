cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}cppserver-example" C CXX)

set(CURL_NO_CURL_CMAKE ON)
find_library(CPP_SERVER REQUIRED NAMES cppserver)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE "${CPP_SERVER}" cppcommon asio ssl crypto fmt uuid ${CMAKE_DL_LIBS})
target_include_directories("${PROJECT_NAME}" PRIVATE "/usr/local/include/CppCommon" "/usr/local/include/CppServer")
