cmake_minimum_required(VERSION 3.5)
project(flat_file)

set(SOURCE_FILES
        flat_file.h
        flat_file.c
        flat_file_types.h
        flat_file_dictionary_handler.h
        flat_file_dictionary_handler.c
        ../../../../../../../Downloads/iondb-development/src/dictionary/dictionary.h
        ../../../../../../../Downloads/iondb-development/src/dictionary/dictionary.c
        ../../../../../../../Downloads/iondb-development/src/dictionary/dictionary_types.h
        ../../../../../../../Downloads/iondb-development/src/key_value/kv_system.h)

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})

    set(${PROJECT_NAME}_SRCS
        ${SOURCE_FILES}
            ../../../../../../../Downloads/iondb-development/src/file/kv_stdio_intercept.h
            ../../../../../../../Downloads/iondb-development/src/file/SD_stdio_c_iface.h
            ../../../../../../../Downloads/iondb-development/src/file/SD_stdio_c_iface.cpp)

#    if(DEBUG)
        set(${PROJECT_NAME}_SRCS
            "${${PROJECT_NAME}_SRCS}"
                ../../../../../../../Downloads/iondb-development/src/serial/printf_redirect.h
                ../../../../../../../Downloads/iondb-development/src/serial/serial_c_iface.h
                ../../../../../../../Downloads/iondb-development/src/serial/serial_c_iface.cpp)
#    endif()

    generate_arduino_library(${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

    # Required on Unix OS family to be able to be linked into shared libraries.
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()