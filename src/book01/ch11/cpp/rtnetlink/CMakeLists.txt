cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}rtnetlink" C CXX)

# set(LINK_SEARCH_START_STATIC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable("${PROJECT_NAME}-mtu" rtnetlink_mtu.cpp)
target_link_libraries("${PROJECT_NAME}-mtu" netlink util_rtnl cap mnl rt)
