cmake_minimum_required(VERSION 3.5)
project(cu_test)

set(SOURCE_FILES
    CuTest.h
    CuTest.c)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)