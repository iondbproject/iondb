cmake_minimum_required(VERSION 3.5)
project(linear_hash_new)

include_directories('.')

set(SOURCE_FILES
        linear_hash_types.h
        linear_hash_file.h
        linear_hash_file.c
        linear_hash_hashing.h
        linear_hash_hashing.c
        linear_hash_macros.h
        linear_hash_util.h
        linear_hash_util.c
        linear_hash.h
        linear_hash_common_types.h
        linear_hash.c
        linear_hash_handler.h
        linear_hash_handler.c
        ../dictionary.h
        ../dictionary.c
        ../dictionary_types.h
        ../../key_value/kv_system.h
        array_list.h
        array_list.c
        )

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})

    set(${PROJECT_NAME}_SRCS
            ${SOURCE_FILES}
            ../../file/kv_stdio_intercept.h
            ../../file/sd_stdio_c_iface.h
            ../../file/sd_stdio_c_iface.cpp)

    #    if(DEBUG)
    set(${PROJECT_NAME}_SRCS
            "${${PROJECT_NAME}_SRCS}"
            ../../serial/printf_redirect.h
            ../../serial/serial_c_iface.h
            ../../serial/serial_c_iface.cpp)
    #    endif()

    generate_arduino_library(${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

    # Required on Unix OS family to be able to be linked into shared libraries.
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()