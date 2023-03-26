project("${CHAPTER_NAME}flatbuffers-example" C CXX)


add_executable("${PROJECT_NAME}" flatbuffers_example.cpp)
target_link_libraries("${PROJECT_NAME}" PRIVATE flatbuffers)
