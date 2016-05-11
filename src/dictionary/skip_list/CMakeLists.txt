cmake_minimum_required(VERSION 3.5)
project(skip_list)

set(SOURCE_FILES
    skip_list.h
    skip_list.c
    skip_list_handler.h
    skip_list_handler.c
    skip_list_types.h
    ../dictionary.h
    ../dictionary.c
    ../dictionary_types.h
    ../../kv_io.h
    ../../kv_io.c
    ../../kv_system.h)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)