cmake_minimum_required(VERSION 3.10)

add_subdirectory("promisc_switcher")
add_subdirectory("raw_sniffer")
add_subdirectory("pcap_sniffer")

if (NOT WIN32)
    add_subdirectory(pcappp-example)
endif()
