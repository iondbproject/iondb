cmake_minimum_required(VERSION 3.5)
project(test_skip_list)

set(SOURCE_FILES
    run_skip_list.c
    test_skip_list.c
    test_skip_list_handler.c
    skip_list_tests.h)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} cu_test skip_list)