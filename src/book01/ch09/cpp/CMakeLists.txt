cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("ancil_data_recv")
#    add_subdirectory("recvmsg")
    add_subdirectory("send_recv_descriptors")
endif()
