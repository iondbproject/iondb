cmake_minimum_required(VERSION 3.5)
project(test_bpp_tree)

set(SOURCE_FILES
    run_bpp_tree.c
    test_bpp_tree_handler.c
    ../generic_dictionary_test.h
    ../generic_dictionary_test.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} planck_unit bpp_tree)