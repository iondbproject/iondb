cmake_minimum_required(VERSION 3.5)
project(test_bpp_tree)

set(SOURCE_FILES
    run_bpp_tree.c
    test_bpp_tree_handler.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} cu_test bpp_tree)