cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}accept-pool" C CXX)

set(${PROJECT_NAME}_SRC main.cpp)

find_package(Boost REQUIRED) # asio

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
target_link_libraries("${PROJECT_NAME}" PRIVATE ${Boost_LIBRARIES} socket-wrapper)
target_include_directories("${PROJECT_NAME}" PRIVATE ${Boost_INCLUDE_DIRS})

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
else()
  target_link_libraries("${PROJECT_NAME}" PRIVATE pthread)
endif()

