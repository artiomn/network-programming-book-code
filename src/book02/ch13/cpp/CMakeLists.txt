cmake_minimum_required(VERSION 3.10)

find_package(Qt6 QUIET)

if (DEFINED Ot6_DIR OR Qt6_DIR)
    add_subdirectory("qt_broadcastsender")
    add_subdirectory("qt_broadcastreceiver")
    add_subdirectory("qt-tcp-server")
    add_subdirectory("qt-interfaces")
else()
    message(WARNING "Please install Qt6 librarires!")
endif()

if (UNIX)
# To build this lesson code on Windows, libraries must be installed manually.
    add_subdirectory("boost_beast_coroutine_htpp_client")
    add_subdirectory(beast_http_req_resp)
    add_subdirectory("gio_client")
    add_subdirectory("gio_server")
    add_subdirectory("poco-examples")
endif()
