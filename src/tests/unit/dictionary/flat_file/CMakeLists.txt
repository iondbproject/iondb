cmake_minimum_required(VERSION 3.5)
project(test_flat_file)

set(SOURCE_FILES
    run_flat_file.c
    test_flat_file.c
    test_flat_file_dictionary_handler.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} planck_unit flat_file)