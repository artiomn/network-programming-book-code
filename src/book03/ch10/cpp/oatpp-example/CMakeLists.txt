cmake_minimum_required(VERSION 3.10)

project("${CHAPTER_NAME}oatpp-example" C CXX)

set(CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME}-lib
        src/AppComponent.hpp
        src/controller/MyController.cpp
        src/controller/MyController.hpp
        src/dto/DTOs.hpp
)

## link libs

find_package(oatpp 1.3.0 REQUIRED)

target_link_libraries(${PROJECT_NAME}-lib
        PUBLIC oatpp::oatpp
        PUBLIC oatpp::oatpp-test
)

target_include_directories(${PROJECT_NAME}-lib PUBLIC src)

## add executables

add_executable(${PROJECT_NAME}-exe
        src/App.cpp
        test/app/MyApiTestClient.hpp)

target_link_libraries(${PROJECT_NAME}-exe ${PROJECT_NAME}-lib)
add_dependencies(${PROJECT_NAME}-exe ${PROJECT_NAME}-lib)

add_executable(${PROJECT_NAME}-test
        test/tests.cpp
        test/app/TestComponent.hpp
        test/app/MyApiTestClient.hpp
        test/MyControllerTest.cpp
        test/MyControllerTest.hpp
)

target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME}-lib)
add_dependencies(${PROJECT_NAME}-test ${PROJECT_NAME}-lib)

set_target_properties(${PROJECT_NAME}-lib ${PROJECT_NAME}-exe ${PROJECT_NAME}-test PROPERTIES
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

enable_testing()
add_test(project-tests ${PROJECT_NAME}-test)
