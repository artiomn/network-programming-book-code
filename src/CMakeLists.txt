cmake_minimum_required(VERSION 3.10)

project(cpp-network-book)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if (MSVC)
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib;.a;${CMAKE_FIND_LIBRARY_SUFFIXES}")
else()
# BUGGY!
#    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;${CMAKE_FIND_LIBRARY_SUFFIXES}")
endif()

set(Boost_USE_STATIC_LIBS ON)
set(LINK_SEARCH_START_STATIC ON)
#set(LINK_SEARCH_END_STATIC ON)
set(BUILD_SHARED_LIBS OFF)
# set(CMAKE_EXE_LINKER_FLAGS "-static")

file(GLOB books LIST_DIRECTORIES true "book[0-9][0-9]")
add_subdirectory("socket_wrapper")

foreach(book ${books})
    message(STATUS "Adding code for the book ${book}")
    get_filename_component(BOOK_NAME "${book}" NAME)
    add_subdirectory("${book}")
endforeach()
