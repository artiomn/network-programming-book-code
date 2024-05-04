project("${CHAPTER_NAME}libp2p-example")

hunter_add_package(libp2p)
find_package(libp2p REQUIRED)

add_executable("${PROJECT_NAME}" "example.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE libp2p)
