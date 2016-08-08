cmake_minimum_required(VERSION 3.5)
project(examples)

set(MASTER_TABLE_SOURCE
		master_table_example.c
		../src/dictionary/ion_master_table.h
		../src/dictionary/ion_master_table.c)

add_executable(example_master_table         ${MASTER_TABLE_SOURCE})
target_link_libraries(example_master_table  bpp_tree flat_file)