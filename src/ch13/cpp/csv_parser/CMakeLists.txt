cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}csv-parser" C CXX)

set(${PROJECT_NAME}_SRC main.cpp)
source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" GTK2::gio)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()

