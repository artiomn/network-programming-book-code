cmake_minimum_required(VERSION 3.10)

add_subdirectory(resolve)
add_subdirectory(ip_convert)

if (NOT WIN32)
	add_subdirectory(namespaces)
	add_subdirectory(net_db)
endif()
