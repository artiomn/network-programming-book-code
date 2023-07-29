cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}incorrect-udp-client" C CXX)

set(${PROJECT_NAME}_SRC main.cpp)

source_group(source FILES ${${PROJECT_NAME}_SRC}})

add_executable("${PROJECT_NAME}" "${${PROJECT_NAME}_SRC}")

#if(WIN32)
#    target_compile_options("${PROJECT_NAME}" PRIVATE /fsanitize=address)
#else()
#    target_compile_options("${PROJECT_NAME}" PRIVATE -fsanitize=address  -fno-omit-frame-pointer)
#    target_compile_definitions("${PROJECT_NAME}" PRIVATE -D_FORTIFY_SOURCE=2)
#    target_link_libraries("${PROJECT_NAME}" asan)
#endif()

target_link_libraries("${PROJECT_NAME}" socket-wrapper)
