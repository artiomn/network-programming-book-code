cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}if_nameindex" C CXX)

add_executable("${PROJECT_NAME}_if_nameindex" if_nameindex.cpp)
add_executable("${PROJECT_NAME}_siocifname" siocifname.cpp)
add_executable("${PROJECT_NAME}_get_ifaddrs" get_ifaddrs.cpp)

target_link_libraries("${PROJECT_NAME}_if_nameindex" socket-wrapper)
target_link_libraries("${PROJECT_NAME}_siocifname" socket-wrapper)
target_link_libraries("${PROJECT_NAME}_get_ifaddrs" socket-wrapper)
