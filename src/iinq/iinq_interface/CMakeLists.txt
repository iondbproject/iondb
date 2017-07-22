cmake_minimum_required(VERSION 3.5)
project(test_iinq_user)

set(SOURCE_FILES
        iinq_user.c
        iinq_user.h
        ../../dictionary/dictionary_types.h
        ../../dictionary/dictionary.c
        ../../dictionary/dictionary.h
        )

add_executable(test_iinq_user         ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} bpp_tree flat_file iinq linear_hash open_address_hash open_address_file_hash skip_list)
#target_link_libraries(test_iinq_user bpp_tree flat_file skip_list open_address_file_hash open_address_hash linear_hash)