project("${CHAPTER_NAME}thrift-example" C CXX)

find_program(THRIFT_COMPILER thrift)

if (NOT THRIFT_COMPILER)
	message(WARNING "Please, install Apache Thrift!")
	return()
endif()

find_package(ZLIB REQUIRED)

# Make sure gen-cpp files can be included
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/gen-cpp")

set("${PROJECT_NAME}_gencpp_SOURCES"
    gen-cpp/phones_types.cpp
    gen-cpp/PhoneService.cpp
)

add_library("${PROJECT_NAME}_thrift_gen" STATIC ${${PROJECT_NAME}_gencpp_SOURCES})
target_link_libraries("${PROJECT_NAME}_thrift_gen" PUBLIC thrift)

add_custom_command(
    OUTPUT
    gen-cpp/PhoneService.cpp
    gen-cpp/PhoneService.h
    gen-cpp/phones_types.cpp
    gen-cpp/phones_types.h
    COMMAND ${THRIFT_COMPILER} --gen cpp -r ${PROJECT_SOURCE_DIR}/../../thrift/phones.thrift
)

add_executable("${PROJECT_NAME}_server" gen-cpp/PhoneService_server.skeleton.cpp)
target_link_libraries("${PROJECT_NAME}_server" PRIVATE "${PROJECT_NAME}_thrift_gen")

add_executable("${PROJECT_NAME}_client" client.cpp)
target_link_libraries("${PROJECT_NAME}_client" PRIVATE "${PROJECT_NAME}_thrift_gen")

if (ZLIB_FOUND)
  target_link_libraries("${PROJECT_NAME}_server" PRIVATE ZLIB::ZLIB)
  target_link_libraries("${PROJECT_NAME}_client" PRIVATE ZLIB::ZLIB)
endif ()
