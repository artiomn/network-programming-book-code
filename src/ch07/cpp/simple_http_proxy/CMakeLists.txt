cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}simple-http-proxy" C CXX)

set(${PROJECT_NAME}_SRC main.cpp proxy_server.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" socket-wrapper)
