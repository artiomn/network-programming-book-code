project("${CHAPTER_NAME}capnp-example" C CXX)

find_package(CapnProto CONFIG REQUIRED)

get_filename_component(${PROJECT_NAME}_PROTO_PATH
                       "${CMAKE_CURRENT_LIST_DIR}/../../capnp"
                       ABSOLUTE)

set(${PROJECT_NAME}_PROTO_FILES
    "echo.capnp"
)

list(TRANSFORM ${PROJECT_NAME}_PROTO_FILES PREPEND "${${PROJECT_NAME}_PROTO_PATH}/")

set(CAPNPC_SRC_PREFIX "${${PROJECT_NAME}_PROTO_PATH}/")

capnp_generate_cpp(ECHO_SOURCE ECHO_HEADERS ${${PROJECT_NAME}_PROTO_FILES})

add_library("${PROJECT_NAME}_GRPC_GEN" OBJECT ${ECHO_SOURCE} ${ECHO_HEADERS})
target_include_directories("${PROJECT_NAME}_GRPC_GEN" PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

add_executable("${PROJECT_NAME}_server" server.cpp)
target_link_libraries("${PROJECT_NAME}_server" PRIVATE CapnProto::capnp-rpc ${PROJECT_NAME}_GRPC_GEN)

add_executable("${PROJECT_NAME}_client" client.cpp)
target_link_libraries("${PROJECT_NAME}_client" PRIVATE CapnProto::capnp-rpc ${PROJECT_NAME}_GRPC_GEN)
