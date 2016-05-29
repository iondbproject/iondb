cmake_minimum_required(VERSION 3.5)
project(test_dictionary)

set(SOURCE_FILES
    ../../../dictionary/ion_master_table.h
    ../../../dictionary/ion_master_table.c
    test_dictionary.h
    test_dictionary.c
    generic_dictionary_test.h
    generic_dictionary_test.c)

if(UseArduino)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})
    set(${PROJECT_NAME}_PORT        ${PORT})
    set(${PROJECT_NAME}_SERIAL      ${SERIAL})

    set(${PROJECT_NAME}_SKETCH      dictionary.ino)

    set(${PROJECT_NAME}_SRCS
        ${SOURCE_FILES}
        ../../../file/kv_stdio_intercept.h
        ../../../file/SD_stdio_c_iface.h
        ../../../file/SD_stdio_c_iface.cpp)

    set(${PROJECT_NAME}_LIBS        planck_unit skip_list)

    generate_arduino_firmware(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME}          ${SOURCE_FILES} run_dictionary.c)

    target_link_libraries(${PROJECT_NAME}   planck_unit skip_list)
endif()