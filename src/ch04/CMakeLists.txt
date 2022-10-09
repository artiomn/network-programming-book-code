cmake_minimum_required(VERSION 3.10)

get_filename_component(CHAPTER_NAME "${CMAKE_CURRENT_LIST_DIR}-" NAME CACHED)

if (UNIX)
    add_subdirectory("ancil_data_recv")
#    add_subdirectory("recvmsg")
#    add_subdirectory("send_recv_descriptors")
endif()
