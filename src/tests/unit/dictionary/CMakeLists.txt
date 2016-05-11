cmake_minimum_required(VERSION 3.5)
project(test_dictionary)

set(SOURCE_FILES
    ../../../dictionary/ion_master_table.h
    ../../../dictionary/ion_master_table.c
    run_dictionary.c
    test_dictionary.c
    generic_dictionary_test.h
    generic_dictionary_test.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} cu_test skip_list)