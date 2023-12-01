cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}phistache-example" C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Pistache REQUIRED IMPORTED_TARGET libpistache)

add_executable("${PROJECT_NAME}-server" "server.cpp")
target_link_libraries("${PROJECT_NAME}-server" PRIVATE ${Pistache_LIBRARIES})
add_executable("${PROJECT_NAME}-client" "client.cpp")
target_link_libraries("${PROJECT_NAME}-client" PRIVATE ${Pistache_LIBRARIES})

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()
