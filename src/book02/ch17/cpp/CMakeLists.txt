cmake_minimum_required(VERSION 3.10)

if (UNIX)
# To build this lesson code on Windows, libraries must be installed manually.
    add_subdirectory("boost_beast_coroutine_htpp_client")
    add_subdirectory("beast_http_req_resp")
endif()
