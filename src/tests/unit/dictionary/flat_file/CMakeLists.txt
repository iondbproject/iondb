cmake_minimum_required(VERSION 3.5)
project(test_flat_file)

set(SOURCE_FILES
    test_flat_file.h
    test_flat_file.c
    test_flat_file_dictionary_handler.h
    test_flat_file_dictionary_handler.c)

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})
    set(${PROJECT_NAME}_PORT        ${PORT})
    set(${PROJECT_NAME}_SERIAL      ${SERIAL})

    set(${PROJECT_NAME}_SKETCH      flat_file.ino)
    set(${PROJECT_NAME}_SRCS        ${SOURCE_FILES})
    set(${PROJECT_NAME}_LIBS        planck_unit flat_file)

    generate_arduino_firmware(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME}          ${SOURCE_FILES} run_flat_file.c)

    target_link_libraries(${PROJECT_NAME}   planck_unit flat_file)
endif()