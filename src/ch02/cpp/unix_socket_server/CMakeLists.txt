cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}unix-socket" C CXX)

set(${PROJECT_NAME}_SRC main.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}-server" server.cpp)
add_executable("${PROJECT_NAME}-client" client.cpp)
