cmake_minimum_required (VERSION 3.10)

project("${CHAPTER_NAME}lwip-runner")

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_C_FLAGS "-O0 -ggdb")
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O0 -ggdb")
set(CMAKE_CXX_FLAGS "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O0 -ggdb")

find_path(PCAP_INCLUDE_DIR NAMES pcap/pcap.h pcap.h HINTS "${PCAP_HINTS}/include")

find_library(PCAP_LIBRARY NAMES pcap wpcap HINTS "${PCAP_HINTS}/lib")

find_path(LWIP_DIR src/include/lwip/init.h ${CMAKE_CURRENT_SOURCE_DIR}/lwip)

include_directories(
	${PCAP_INCLUDE_DIR}
	${LWIP_DIR}/src/include
    ${LWIP_DIR}/contrib/ports/unix/port
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/lwip-include
)

include(${LWIP_DIR}/src/Filelists.cmake)

add_executable("${PROJECT_NAME}"
    ${LWIP_DIR}/contrib/ports/unix/port/sys_arch.c
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/echo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ping.cpp
)

target_link_libraries("${PROJECT_NAME}" "${PCAP_LIBRARY}" lwipcore pthread)
