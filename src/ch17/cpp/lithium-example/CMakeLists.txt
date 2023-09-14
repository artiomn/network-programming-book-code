cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}lithium-example" C CXX)

# Header-only library.
# find_library(CPP_SERVER REQUIRED NAMES lithium)

add_executable("${PROJECT_NAME}" "main.cpp")
