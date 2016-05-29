cmake_minimum_required(VERSION 3.5)
project(linear_hash)

set(SOURCE_FILES
    file_linked_list.h
    file_linked_list.c
    linear_hash.h
    linear_hash.c
    linear_hash_dictionary.h
    linear_hash_dictionary_handler.h
    linear_hash_dictionary_handler.c
    linear_hash_types.h
    ../../util/id_encoder/file_encoder.h
    ../../util/id_encoder/file_encoder.c
    ../dictionary.h
    ../dictionary.c
    ../dictionary_types.h
    ../../kv_io.h
    ../../kv_io.c
    ../../kv_system.h)

if(UseArduino)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})

    set(${PROJECT_NAME}_SRCS
        ${SOURCE_FILES}
        ../../file/kv_stdio_intercept.h
        ../../file/SD_stdio_c_iface.h
        ../../file/SD_stdio_c_iface.cpp)

    if(DEBUG)
        set(${PROJECT_NAME}_SRCS "${PROJECT_NAME}_SRCS
            ../../serial/printf_redirect.h
            ../../serial/serial_c_iface.h
            ../../serial/serial_c_iface.cpp")
    endif()

    generate_arduino_library(${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

    # Required on Unix OS family to be able to be linked into shared libraries.
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()