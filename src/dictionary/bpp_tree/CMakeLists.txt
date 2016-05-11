cmake_minimum_required(VERSION 3.5)
project(bpp_tree)

set(SOURCE_FILES
    bpp_tree.h
    bpp_tree.c
    bpp_tree_handler.h
    bpp_tree_handler.c
    ../../file/linked_file_bag.h
    ../../file/linked_file_bag.c
    ../../file/ion_file.h
    ../../file/ion_file.c
    ../dictionary.h
    ../dictionary.c
    ../dictionary_types.h
    ../../kv_io.h
    ../../kv_io.c
    ../../kv_system.h)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)