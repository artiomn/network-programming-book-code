cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}poco-examples" C CXX)

# include(FetchContent)
# FetchContent_Declare(
#   Poco
#   URL https://github.com/pocoproject/poco/archive/refs/tags/poco-1.10.1-release.zip
# )

find_package(Poco COMPONENTS Net REQUIRED)

add_executable("${PROJECT_NAME}-dns-resolver" "dns_resolver.cpp")
add_executable("${PROJECT_NAME}-client" "client.cpp")
add_executable("${PROJECT_NAME}-server" "server.cpp")
target_link_libraries("${PROJECT_NAME}-dns-resolver" PRIVATE Poco::Net)
target_link_libraries("${PROJECT_NAME}-server" PRIVATE Poco::Net)
target_link_libraries("${PROJECT_NAME}-client" PRIVATE Poco::Net)

