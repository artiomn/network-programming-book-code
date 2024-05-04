cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("libtorrent")
    add_subdirectory("simple_blockchain")
#    add_subdirectory("libp2p")
endif()
