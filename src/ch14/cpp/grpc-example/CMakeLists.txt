project("${CHAPTER_NAME}grpc-example" C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)

get_filename_component(${PROJECT_NAME}_PROTO_PATH
                       "${CMAKE_CURRENT_LIST_DIR}/../../grpc"
                       ABSOLUTE)

set(${PROJECT_NAME}_PROTO_FILES
    "search_service.proto"
)

foreach(_f ${${PROJECT_NAME}_PROTO_FILES})
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_f}"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${${PROJECT_NAME}_PROTO_PATH}/${_f}"
            "${CMAKE_CURRENT_BINARY_DIR}/${_f}"
        DEPENDS "${${PROJECT_NAME}_PROTO_PATH}/${_f}"
    )
endforeach()

list(TRANSFORM ${PROJECT_NAME}_PROTO_FILES PREPEND "${CMAKE_CURRENT_BINARY_DIR}/")

add_library("${PROJECT_NAME}_GRPC_GEN" OBJECT ${${PROJECT_NAME}_PROTO_FILES})

target_link_libraries("${PROJECT_NAME}_GRPC_GEN" PUBLIC
    protobuf::libprotobuf
    gRPC::grpc++
)

target_include_directories("${PROJECT_NAME}_GRPC_GEN" PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")
target_include_directories("${PROJECT_NAME}_GRPC_GEN" PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

protobuf_generate(
    TARGET ${PROJECT_NAME}_GRPC_GEN
    IMPORT_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
    PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
)

protobuf_generate(
    TARGET ${PROJECT_NAME}_GRPC_GEN
    LANGUAGE grpc
    IMPORT_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
    PROTOC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=\$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
)

add_executable("${PROJECT_NAME}_server" server.cpp)
target_link_libraries("${PROJECT_NAME}_server" PRIVATE "${PROJECT_NAME}_GRPC_GEN")

add_executable("${PROJECT_NAME}_client" client.cpp)
target_link_libraries("${PROJECT_NAME}_client" PRIVATE "${PROJECT_NAME}_GRPC_GEN")

#if (ZLIB_FOUND)
#  target_link_libraries("${PROJECT_NAME}_server" PRIVATE ZLIB::ZLIB)
#  target_link_libraries("${PROJECT_NAME}_client" PRIVATE ZLIB::ZLIB)
#endif ()
