cmake_minimum_required(VERSION 3.5)
project(id_encoder)

set(SOURCE_FILES
    file_encoder.h
    file_encoder.c
    ../../dictionary/dictionary_types.h
    ../../kv_io.h
    ../../kv_io.c
    ../../kv_system.h)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)