cmake_minimum_required(VERSION 3.10)

project(cpp-network-lesson-4 C CXX)

add_subdirectory("reuse_addr_experiment")
add_subdirectory("mt_server")
add_subdirectory("select_aio_server")

if (UNIX)
	add_subdirectory("libevent_echo_server")
	add_subdirectory("libevpp_echo_server")
	add_subdirectory("libuv_echo_server")
	add_subdirectory("c_ares_example")
	add_subdirectory("asio_server")
	add_subdirectory("oob_example")
endif()
