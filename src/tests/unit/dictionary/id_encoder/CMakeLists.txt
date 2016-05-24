cmake_minimum_required(VERSION 3.5)
project(test_id_encoder)

set(SOURCE_FILES
    run_file_encoder.c
    test_file_encoder.c)

if(UseArduino)
    set(${PROJECT_NAME}_BOARD ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL ${MANUAL})
    set(${PROJECT_NAME}_PORT ${PORT})
    set(${PROJECT_NAME}_SERIAL ${SERIAL})

    set(${PROJECT_NAME}_SKETCH id_encoder.ino)
    set(${PROJECT_NAME}_SRCS ${SOURCE_FILES})
    set(${PROJECT_NAME}_LIBS planck_unit id_encoder)

    generate_arduino_firmware(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME} ${SOURCE_FILES})

    target_link_libraries(${PROJECT_NAME} planck_unit id_encoder)
endif()