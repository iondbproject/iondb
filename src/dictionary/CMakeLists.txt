cmake_minimum_required(VERSION 3.5)
#project(dictionary)

set(DICTIONARY
    ${CMAKE_CURRENT_SOURCE_DIR}/dictionary.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dictionary.c
        ${CMAKE_CURRENT_SOURCE_DIR}/dictionary_types.h)

#add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

# Required on Unix OS family to be able to be linked into shared libraries.
#set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

#target_link_libraries(${PROJECT_NAME} kv_system)