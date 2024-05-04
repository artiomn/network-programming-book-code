cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}webtoolkit-example" C CXX)

find_package (Wt REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" Wt::Wt Wt::HTTP)
