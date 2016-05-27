cmake_minimum_required(VERSION 3.5)
project(flat_file)

set(SOURCE_FILES
    flat_file.h
    flat_file.c
    file_handler.h
    flat_file_dictionary.h
    flat_file_dictionary_handler.h
    flat_file_dictionary_handler.c
    ../dictionary.h
    ../dictionary.c
    ../dictionary_types.h
    ../../kv_io.h
    ../../kv_io.c
    ../../kv_system.h)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)