project("${CHAPTER_NAME}libtorrent-examples")

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
	add_compile_options(-Wno-implicit-int-float-conversion)
endif()

add_executable("${CHAPTER_NAME}magnet" "magnet.cpp")
target_link_libraries("${CHAPTER_NAME}magnet" PRIVATE torrent-rasterbar)

add_executable("${CHAPTER_NAME}torrent-client" "client.cpp")
target_link_libraries("${CHAPTER_NAME}torrent-client" PRIVATE torrent-rasterbar)

