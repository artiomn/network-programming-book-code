cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}modbus-example" C CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Modbus IMPORTED_TARGET libmodbus REQUIRED)

add_executable("${PROJECT_NAME}-read-register" "master-read-register.cpp")
target_link_libraries("${PROJECT_NAME}-read-register" PRIVATE PkgConfig::Modbus)

add_executable("${PROJECT_NAME}-master" "master-tcp.cpp")
target_link_libraries("${PROJECT_NAME}-master" PRIVATE PkgConfig::Modbus)

add_executable("${PROJECT_NAME}-slave" "slave-tcp.cpp")
target_link_libraries("${PROJECT_NAME}-slave" PRIVATE PkgConfig::Modbus)
