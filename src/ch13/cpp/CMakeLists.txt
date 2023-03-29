cmake_minimum_required(VERSION 3.10)

add_subdirectory("qt_broadcastsender")
add_subdirectory("qt_broadcastreceiver")
add_subdirectory("qt-tcp-server")

if (UNIX)
# To build this lesson code on Windows, libraries must be installed manually.
    add_subdirectory("boost_beast_coroutine_htpp_client")
    add_subdirectory("gio_client")
    add_subdirectory("gio_server")
    add_subdirectory("poco-examples")
endif()
