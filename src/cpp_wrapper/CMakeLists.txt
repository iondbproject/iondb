cmake_minimum_required(VERSION 3.5)
project(cpp_wrapper)

set(SOURCE_FILES
	../dictionary/ion_master_table.h
	../dictionary/ion_master_table.c)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(
	${PROJECT_NAME}
	INTERFACE
	bpp_tree
	flat_file
	linear_hash
	open_address_file_hash
	open_address_hash
	skip_list)