cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}qt-tcp-server" C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Network Widgets REQUIRED)

set(${PROJECT_NAME}_SRC main.cpp tcp_server.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" PRIVATE Qt6::Core Qt6::Network Qt::Widgets)

add_executable("${CHAPTER_NAME}qt-tcp-simple-server" "simple_variant.cpp")
target_link_libraries("${CHAPTER_NAME}qt-tcp-simple-server" PRIVATE Qt6::Core Qt6::Network Qt::Widgets)
