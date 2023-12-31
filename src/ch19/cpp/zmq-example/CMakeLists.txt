cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}zmq-example" C CXX)

find_package(cppzmq REQUIRED)

add_executable("${PROJECT_NAME}-push-pull" "push-pull.cpp")
target_link_libraries("${PROJECT_NAME}-push-pull" PRIVATE cppzmq)

add_executable("${PROJECT_NAME}-rep-server" "rep-server.cpp")
target_link_libraries("${PROJECT_NAME}-rep-server" PRIVATE cppzmq)

add_executable("${PROJECT_NAME}-req-client" "req-client.cpp")
target_link_libraries("${PROJECT_NAME}-req-client" PRIVATE cppzmq)

add_executable("${PROJECT_NAME}-router" "router.cpp")
target_link_libraries("${PROJECT_NAME}-router" PRIVATE cppzmq)
