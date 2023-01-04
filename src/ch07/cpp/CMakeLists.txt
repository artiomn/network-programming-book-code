cmake_minimum_required(VERSION 3.10)

add_subdirectory("promisc_switcher")
add_subdirectory("raw_sniffer")
add_subdirectory("pcap_sniffer")
add_subdirectory("simple_http_proxy")

if (UNIX)
    add_subdirectory("tun_example")
endif()
