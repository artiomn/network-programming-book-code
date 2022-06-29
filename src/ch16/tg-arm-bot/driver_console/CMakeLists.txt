cmake_minimum_required(VERSION 3.1)

set(project_name tg-arm-bot-driver-console)

project(${project_name})

set(CMAKE_CXX_STANDARD 11)

include_directories(src)


add_library(${project_name}-lib
    src/WSListener.cpp
    src/WSListener.hpp
)

## link libs

find_package(oatpp              1.3.0 REQUIRED)
find_package(oatpp-websocket    1.3.0 REQUIRED)

target_link_libraries(${project_name}-lib
    PUBLIC oatpp::oatpp
    PUBLIC oatpp::oatpp-websocket
)

add_executable(${project_name}
    src/App.cpp
)
target_link_libraries(${project_name} ${project_name}-lib)
add_dependencies(${project_name} ${project_name}-lib)

set_target_properties(${project_name}-lib ${project_name} PROPERTIES
    CXX_STANDARD 11
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
    LINKER_LANGUAGE CXX
)

