cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}microhttpd-example" C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(microhttpd REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE "${MICROHTTPD_LIBRARIES}")

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()
