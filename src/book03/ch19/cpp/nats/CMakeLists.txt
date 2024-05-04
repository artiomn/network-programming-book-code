cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}nats-example" C CXX)

find_library(NATS_C nats REQUIRED)

add_executable("${PROJECT_NAME}-publisher" "publish.cpp")
target_link_libraries("${PROJECT_NAME}-publisher" PRIVATE "${NATS_C}")

add_executable("${PROJECT_NAME}-subscriber" "subscribe.cpp")
target_link_libraries("${PROJECT_NAME}-subscriber" PRIVATE "${NATS_C}")
