cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}mail-slot" C CXX)

add_executable("${PROJECT_NAME}-client" client.cpp)
add_executable("${PROJECT_NAME}-server" server.cpp)
