project(libtorrent-examples)

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
	add_compile_options(-Wno-implicit-int-float-conversion)
endif()

add_executable(magnet "magnet.cpp")
target_link_libraries(magnet PRIVATE torrent-rasterbar)

add_executable(torrent-client "client.cpp")
target_link_libraries(torrent-client PRIVATE torrent-rasterbar)

