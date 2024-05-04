project(network-${BOOK_NAME}-examples)

file(GLOB chapters LIST_DIRECTORIES true "ch[0-9][0-9]")

foreach(chapter ${chapters})
    message(STATUS "Adding code for the chapter ${chapter}")
    get_filename_component(CHAPTER_NAME "${chapter}" NAME)
    string(REGEX REPLACE "^[^0-9]+" "b" BOOK_NAME "${BOOK_NAME}")
    set(CHAPTER_NAME "${BOOK_NAME}-${CHAPTER_NAME}-")
    add_subdirectory("${chapter}/cpp")
endforeach()
