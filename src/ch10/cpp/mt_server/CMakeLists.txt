cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}mt-server" C CXX)

set(${PROJECT_NAME}_SRC main.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" PRIVATE socket-wrapper)

if(NOT WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE pthread)
endif()
