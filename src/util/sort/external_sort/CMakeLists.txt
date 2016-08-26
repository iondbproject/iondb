cmake_minimum_required(VERSION 3.5)
project(external_sort)

set(SOURCE_FILES
        external_sort.h
        external_sort.c
        external_sort_types.h
        flash_min_sort.h
        flash_min_sort.c
        external_merge_sort.h
        external_merge_sort.c
        ../sort.h
        ../../../key_value/kv_system.h)

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})

    set(${PROJECT_NAME}_SRCS
            ${SOURCE_FILES}
            ../../../serial/printf_redirect.h
            ../../../serial/serial_c_iface.h
            ../../../serial/serial_c_iface.cpp
            ../../../file/kv_stdio_intercept.h
            ../../../file/SD_stdio_c_iface.h
            ../../../file/SD_stdio_c_iface.cpp)

    generate_arduino_library(${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

    # Required on Unix OS family to be able to be linked into shared libraries.
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()