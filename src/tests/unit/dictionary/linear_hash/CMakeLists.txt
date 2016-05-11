cmake_minimum_required(VERSION 3.5)
project(test_linear_hash)

set(SOURCE_FILES
    run_linear_hash.c
    test_linear_hash.c
    test_linear_hash_dictionary.c
    test_linear_hash_dictionary_handler.c
    test_file_linked_list.c
    ../generic_dictionary_test.h
    ../generic_dictionary_test.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} cu_test linear_hash)