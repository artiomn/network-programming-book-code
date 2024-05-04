cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}mqtt-example" C CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Mosquitto IMPORTED_TARGET libmosquitto REQUIRED)

add_executable("${PROJECT_NAME}" "mosquitto-example.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE PkgConfig::Mosquitto)
