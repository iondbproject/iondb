cmake_minimum_required(VERSION 3.5)
project(examples)

set(HASH_MAP_SOURCE
    hash_map_example.c)

set(LFSR_SOURCE
    lfsr_example.c)

set(SKIP_LIST_SOURCE
    skip_list_example.c)

set(MASTER_TABLE_SOURCE
    master_table_example.c
    ../dictionary/ion_master_table.h
    ../dictionary/ion_master_table.c)

add_executable(example_hash_map             ${HASH_MAP_SOURCE})
target_link_libraries(example_hash_map      open_address_hash)

add_executable(example_lfsr                 ${LFSR_SOURCE})
target_link_libraries(example_lfsr          lfsr)

add_executable(example_skip_list            ${SKIP_LIST_SOURCE})
target_link_libraries(example_skip_list     skip_list)

add_executable(example_master_table         ${MASTER_TABLE_SOURCE})
target_link_libraries(example_master_table  bpp_tree)