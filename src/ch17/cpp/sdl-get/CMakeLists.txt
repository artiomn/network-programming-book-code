cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}sdl-net-example" C CXX)

find_package(SDL2 REQUIRED)
find_package(SDL2_net REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" SDL2 SDL2_net)
