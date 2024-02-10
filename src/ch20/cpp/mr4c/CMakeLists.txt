project("${CHAPTER_NAME}mr4c-example")

add_executable("${PROJECT_NAME}" "main.cpp")
target_link_libraries("${PROJECT_NAME}" PRIVATE mr4c)
