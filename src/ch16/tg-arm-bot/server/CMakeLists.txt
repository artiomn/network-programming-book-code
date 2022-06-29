cmake_minimum_required(VERSION 3.10)

project(tg-arm-bot-server C CXX)


set(Boost_USE_MULTITHREADED ON)

find_package(Threads REQUIRED)
find_library(RESTBED restbed REQUIRED PATHS "/usr/local/library")

# Temporary hack.
set(RESTBED /usr/local/library/librestbed.a)

find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(CURL)
find_library(TG_BOT TgBot REQUIRED)

if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()


file(GLOB_RECURSE ${PROJECT_NAME}_SERVER_SOURCE robotic_arm_server_api/*.cpp)

add_executable("${PROJECT_NAME}"
               main.cpp
               business_logic.cpp
               server_tg.cpp
               server_web.cpp
               web_socket.cpp
               ${${PROJECT_NAME}_SERVER_SOURCE})

target_include_directories("${PROJECT_NAME}" PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/robotic_arm_server_api/api
    ${CMAKE_CURRENT_LIST_DIR}/robotic_arm_server_api/model
    ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
target_link_libraries("${PROJECT_NAME}" PRIVATE
    ${RESTBED}
    OpenSSL::SSL OpenSSL::Crypto
    "${TG_BOT}" ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES} ${CURL_LIBRARIES})

