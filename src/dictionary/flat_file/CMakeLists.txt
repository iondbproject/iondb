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
        ../../key_value/kv_io.h
        ../../key_value/kv_io.c
        ../../key_value/kv_system.h)

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})

    set(${PROJECT_NAME}_SRCS
        ${SOURCE_FILES}
        ../../file/kv_stdio_intercept.h
        ../../file/SD_stdio_c_iface.h
        ../../file/SD_stdio_c_iface.cpp)

#    if(DEBUG)
        set(${PROJECT_NAME}_SRCS
            "${${PROJECT_NAME}_SRCS}"
            ../../serial/printf_redirect.h
            ../../serial/serial_c_iface.h
            ../../serial/serial_c_iface.cpp)
#    endif()

    set(${PROJECT_NAME}_LIBS bpp_tree)

    generate_arduino_library(${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

    target_link_libraries(${PROJECT_NAME} bpp_tree)

    # Required on Unix OS family to be able to be linked into shared libraries.
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()