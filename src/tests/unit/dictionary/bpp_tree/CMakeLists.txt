cmake_minimum_required(VERSION 3.5)
project(test_bpp_tree)

set(SOURCE_FILES
    run_bpp_tree.c
    test_bpp_tree_handler.c
    ../generic_dictionary_test.h
    ../generic_dictionary_test.c)

if(UseArduino)
    set(${PROJECT_NAME}_BOARD ${BOARD})
    #    set(${PROJECT_NAME}_MANUAL ${MANUAL})

    set(${PROJECT_NAME}_SKETCH bpp_tree.ino)
    set(${PROJECT_NAME}_SRCS ../../../../serial/serial_c_iface.h ../../../../serial/printf_redirect.h ${SOURCE_FILES})
    set(${PROJECT_NAME}_LIBS planck_unit bpp_tree)

    GENERATE_ARDUINO_FIRMWARE(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME} run_bpp_tree.c ${SOURCE_FILES})

    target_link_libraries(${PROJECT_NAME} planck_unit bpp_tree)
endif()