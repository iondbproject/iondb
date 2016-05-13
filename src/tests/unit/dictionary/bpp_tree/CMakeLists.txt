cmake_minimum_required(VERSION 3.5)
project(test_bpp_tree)

set(SOURCE_FILES
#    test_bpp_tree_handler.h
    test_bpp_tree_handler.c)

if(UseArduino)
    set(${PROJECT_NAME}_BOARD ${BOARD})
#    set(${PROJECT_NAME}_MANUAL ${MANUAL})

    set(${PROJECT_NAME}_SKETCH bpp_tree.ino)
    set(${PROJECT_NAME}_SRCS ../../../../serial/serial_c_iface.h ../../../../serial/printf_redirect.h ${SOURCE_FILES})
    set(${PROJECT_NAME}_LIBS cu_test bpp_tree)

    GENERATE_ARDUINO_FIRMWARE(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME} run_bpp_tree.c ${SOURCE_FILES})

    target_link_libraries(${PROJECT_NAME} cu_test bpp_tree)
endif()