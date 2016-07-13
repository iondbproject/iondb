cmake_minimum_required(VERSION 3.5)
project(iinq)

set(SOURCE_FILES
    ../dictionary/ion_master_table.h
    ../dictionary/ion_master_table.c
    iinq.h
    iinq.c)

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})

    set(${PROJECT_NAME}_SRCS        ${SOURCE_FILES})

    set(${PROJECT_NAME}_LIBS        bpp_tree)

    generate_arduino_library(${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

    target_link_libraries(${PROJECT_NAME}   bpp_tree)

    # Required on Unix OS family to be able to be linked into shared libraries.
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()