cmake_minimum_required(VERSION 3.5)
project(test_open_address_file_hash)

set(SOURCE_FILES
        run_open_address_file_hash.c
        test_open_address_file_hash.c
        test_open_address_file_hash_dictionary_handler.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} planck_unit open_address_file_hash)