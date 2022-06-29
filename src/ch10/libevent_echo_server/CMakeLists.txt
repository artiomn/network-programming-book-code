cmake_minimum_required(VERSION 3.10)

project(libevent-echo-server C CXX)

# Add pkg-config functionality
find_package(PkgConfig REQUIRED)

# search for libevent.pc
pkg_search_module(EVENT REQUIRED libevent)

# This will works not on the all distributions.
# find_package(Libevent REQUIRED COMPONENTS core)
set(${PROJECT_NAME}_SRC libevent_echo_server.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")
include_directories(${LIBEVENT_INCLUDE_DIRS})

# Alternative for the "find_package(Libevent)" variant.
# target_link_libraries("${PROJECT_NAME}" libevent::core)
target_link_libraries("${PROJECT_NAME}" event)


if(WIN32)
  target_link_libraries("${PROJECT_NAME}" wsock32 ws2_32)
endif()

