cmake_minimum_required(VERSION 3.5)
project(test_open_address_hash)

set(SOURCE_FILES
    test_open_address_hash.h
    test_open_address_hash.c
    test_open_address_hash_dictionary_handler.h
    test_open_address_hash_dictionary_handler.c)

if(UseArduino)
    set(${PROJECT_NAME}_BOARD ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL ${MANUAL})
    set(${PROJECT_NAME}_PORT ${PORT})
    set(${PROJECT_NAME}_SERIAL ${SERIAL})

    set(${PROJECT_NAME}_SKETCH open_address_hash.ino)
    set(${PROJECT_NAME}_SRCS ${SOURCE_FILES})
    set(${PROJECT_NAME}_LIBS planck_unit open_address_hash)

    generate_arduino_firmware(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})

    target_link_libraries(${PROJECT_NAME} planck_unit open_address_hash)
endif()