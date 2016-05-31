cmake_minimum_required(VERSION 3.5)
project(test_linear_hash)

set(SOURCE_FILES
    test_linear_hash.h
    test_linear_hash.c
    test_linear_hash_dictionary.h
    test_linear_hash_dictionary.c
    test_linear_hash_dictionary_handler.h
    test_linear_hash_dictionary_handler.c
    test_file_linked_list.h
    test_file_linked_list.c
    ../generic_dictionary_test.h
    ../generic_dictionary_test.c)

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})
    set(${PROJECT_NAME}_PORT        ${PORT})
    set(${PROJECT_NAME}_SERIAL      ${SERIAL})

    set(${PROJECT_NAME}_SKETCH      linear_hash.ino)
    set(${PROJECT_NAME}_SRCS        ${SOURCE_FILES})
    set(${PROJECT_NAME}_LIBS        planck_unit linear_hash)

    generate_arduino_firmware(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME}          ${SOURCE_FILES} run_linear_hash.c)

    target_link_libraries(${PROJECT_NAME}   planck_unit linear_hash m)
endif()