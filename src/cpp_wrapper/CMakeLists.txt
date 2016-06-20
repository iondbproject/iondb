cmake_minimum_required(VERSION 3.5)
project(cpp_wrapper)

add_library(${PROJECT_NAME} INTERFACE)

target_link_libraries(
		${PROJECT_NAME}
		INTERFACE
		bpp_tree
		flat_file
		linear_hash
		open_address_file_hash
		open_address_hash
		skip_list)