cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}lithium-example" C CXX)

# Header-only library.
# find_library(CPP_SERVER REQUIRED NAMES lithium)

find_package(Boost REQUIRED COMPONENTS system context) # asio
find_package(OpenSSL REQUIRED)

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
