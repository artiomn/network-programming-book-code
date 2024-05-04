cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}httpserver-example" C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(LibHttpServer REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE "${LIBHTTPSERVER_LIBRARIES}")

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()
