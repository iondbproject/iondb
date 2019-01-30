cmake_minimum_required(VERSION 3.5)
project(block_write_benchmarks)

set(SOURCE_FILES)

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})
    set(${PROJECT_NAME}_PORT        ${PORT})
    set(${PROJECT_NAME}_SERIAL      ${SERIAL})

    set(${PROJECT_NAME}_SKETCH block_write_benchmarks.ino)
    set(${PROJECT_NAME}_SRCS
            ${SOURCE_FILES}
            )
    #    if(DEBUG)
    set(${PROJECT_NAME}_SRCS
            "${${PROJECT_NAME}_SRCS}"
            )
    #    endif()

    set(${PROJECT_NAME}_LIBS        )

    generate_arduino_firmware(${PROJECT_NAME})
endif()