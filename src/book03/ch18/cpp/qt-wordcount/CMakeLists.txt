cmake_minimum_required(VERSION 3.16)
project("${CHAPTER_NAME}qt-wordcount" LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Concurrent Core Widgets)

add_executable("${PROJECT_NAME}" main.cpp)

target_link_libraries("${PROJECT_NAME}" PRIVATE
    Qt::Concurrent
    Qt::Core
    Qt::Widgets
)

