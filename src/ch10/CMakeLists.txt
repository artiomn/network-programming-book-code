cmake_minimum_required(VERSION 3.10)

get_filename_component(CHAPTER_NAME "${CMAKE_CURRENT_LIST_DIR}-" NAME CACHED)

add_subdirectory("select_aio_server")

if (UNIX)
    add_subdirectory("libevent_echo_server")
    add_subdirectory("libevpp_echo_server")
    add_subdirectory("libuv_echo_server")
    add_subdirectory("c_ares_example")
    add_subdirectory("asio_server")
endif()
