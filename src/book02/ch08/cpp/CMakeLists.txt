cmake_minimum_required(VERSION 3.10)

if (UNIX)
    add_subdirectory("asio_buffers")
    add_subdirectory("asio_sync_server")
    add_subdirectory("asio_simple_async_server")
    add_subdirectory("asio_server")
    add_subdirectory("asio_resolver")
endif()
