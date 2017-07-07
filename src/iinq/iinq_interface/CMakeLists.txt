cmake_minimum_required(VERSION 3.5)
project(test_iinq_user)

set(IINQ_USER_SOURCE
        iinq_user.c
        iinq_user.h
        ../../dictionary/dictionary_types.h
        )

add_executable(test_iinq_user         ${IINQ_USER_SOURCE})
#target_link_libraries(test_iinq_user bpp_tree flat_file skip_list open_address_file_hash open_address_hash linear_hash)