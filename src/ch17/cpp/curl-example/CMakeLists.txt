cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}curl-example" C CXX)

set(CURL_NO_CURL_CMAKE ON)
find_package(CURL REQUIRED COMPONENTS HTTP HTTPS)

#add_executable("${PROJECT_NAME}" "main.cpp")
#target_link_libraries("${PROJECT_NAME}" PRIVATE CURL::libcurl)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()
