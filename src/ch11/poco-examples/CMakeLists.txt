cmake_minimum_required(VERSION 3.10)

project(poco-example C CXX)

# include(FetchContent)
# FetchContent_Declare(
#   Poco
#   URL https://github.com/pocoproject/poco/archive/refs/tags/poco-1.10.1-release.zip
# )

find_package(Poco COMPONENTS Net REQUIRED)

add_executable("poco-dns-resolver" "dns_resolver.cpp")
add_executable("poco-client" "client.cpp")
add_executable("poco-server" "server.cpp")
target_link_libraries("poco-dns-resolver" PRIVATE Poco::Net)
target_link_libraries("poco-server" PRIVATE Poco::Net)
target_link_libraries("poco-client" PRIVATE Poco::Net)

if(WIN32)
  target_link_libraries("${PROJECT_NAME}" PRIVATE wsock32 ws2_32)
endif()

