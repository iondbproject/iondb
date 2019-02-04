cmake_minimum_required(VERSION 3.5)
project(block_write_benchmarks)

set(SOURCE_FILES
        block_write_benchmarks.h
        block_write_benchmarks.c
        ../../file/sd_stdio_c_iface.h
        ../../file/kv_stdio_intercept.h
        )

if (USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL ${MANUAL})
    set(${PROJECT_NAME}_PORT ${PORT})
    set(${PROJECT_NAME}_SERIAL ${SERIAL})

    set(${PROJECT_NAME}_SKETCH block_write_benchmarks.ino)
    set(${PROJECT_NAME}_SRCS
            ${SOURCE_FILES}
            )
    #    if(DEBUG)
    set(${PROJECT_NAME}_SRCS
            "${${PROJECT_NAME}_SRCS}"
            )
    #    endif()

    set(${PROJECT_NAME}_LIBS)

    generate_arduino_firmware(${PROJECT_NAME})
else ()
    add_executable(${PROJECT_NAME} ${SOURCE_FILES} run_block_write_benchmarks.c)

    target_link_libraries(${PROJECT_NAME} planck_unit)

    # Use cmake -DCOVERAGE_TESTING=ON to include coverage testing information.
    if (CMAKE_COMPILER_IS_GNUCC AND COVERAGE_TESTING)
        set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
        set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
    endif ()
endif ()