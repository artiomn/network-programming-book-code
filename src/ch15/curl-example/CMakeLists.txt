cmake_minimum_required(VERSION 3.10)

project(curl-example C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CURL_NO_CURL_CMAKE ON)
find_package(CURL REQUIRED COMPONENTS HTTP HTTPS)

#add_executable("${PROJECT_NAME}" "main.cpp")
#target_link_libraries("${PROJECT_NAME}" PRIVATE CURL::libcurl)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()

