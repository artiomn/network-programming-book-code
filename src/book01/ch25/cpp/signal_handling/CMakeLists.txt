cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}signal-handling" C CXX)

add_executable("${CHAPTER_NAME}sigchld_handler" "sigchld_handler.cpp")
