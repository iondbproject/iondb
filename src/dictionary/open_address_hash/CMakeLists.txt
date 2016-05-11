cmake_minimum_required(VERSION 3.5)
project(open_address_hash)

set(SOURCE_FILES
        open_address_hash.h
        open_address_hash.c
        open_address_hash_dictionary.h
        open_address_hash_dictionary_handler.h
        open_address_hash_dictionary_handler.c
    ../dictionary.h
    ../dictionary.c
    ../dictionary_types.h
    ../../kv_io.h
    ../../kv_io.c
    ../../kv_system.h)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)