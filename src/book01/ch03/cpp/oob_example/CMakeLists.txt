cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}oob-server" C CXX)

add_executable("${PROJECT_NAME}" oob_server.cpp)
target_link_libraries("${PROJECT_NAME}" socket-wrapper)

add_executable("${PROJECT_NAME}-async" oob_server_async.cpp)
target_link_libraries("${PROJECT_NAME}-async" socket-wrapper)
