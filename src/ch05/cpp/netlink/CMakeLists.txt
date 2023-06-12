cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}netlink" C CXX)

add_executable("${PROJECT_NAME}-uevent" uevent.cpp)
add_executable("${PROJECT_NAME}-mtu" mtu.cpp)
