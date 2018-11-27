cmake_minimum_required(VERSION 3.5)
project(benchmark_linear_hash_new)

set(SOURCE_FILES
        linear_hash_benchmarks.h
        linear_hash_benchmarks.c
        ../../tests/planck-unit/src/ion_time/ion_time.h
        ../../tests/planck-unit/src/ion_time/ion_time.c
        ../../util/lfsr/lfsr.h
        ../../util/lfsr/lfsr.c
        )

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})
    set(${PROJECT_NAME}_PORT        ${PORT})
    set(${PROJECT_NAME}_SERIAL      ${SERIAL})

    set(${PROJECT_NAME}_SKETCH linear_hash_benchmarks.ino)
    set(${PROJECT_NAME}_SRCS
            ${SOURCE_FILES}
            linear_hash_benchmarks.c
            linear_hash_benchmarks.h
            ../../tests/planck-unit/src/ion_time/ion_time.c
            ../../file/kv_stdio_intercept.h
            ../../file/sd_stdio_c_iface.h
            ../../file/sd_stdio_c_iface.cpp
            )
    #    if(DEBUG)
    set(${PROJECT_NAME}_SRCS
            "${${PROJECT_NAME}_SRCS}"
            ../../serial/printf_redirect.h
            ../../serial/serial_c_iface.h
            ../../serial/serial_c_iface.cpp)
    #    endif()

    set(${PROJECT_NAME}_LIBS        linear_hash_new flat_file)

    generate_arduino_firmware(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME}          ${SOURCE_FILES} run_linear_hash_benchmarks.c)

    target_link_libraries(${PROJECT_NAME}   planck_unit linear_hash_new flat_file)

    # Use cmake -DCOVERAGE_TESTING=ON to include coverage testing information.
    if (CMAKE_COMPILER_IS_GNUCC AND COVERAGE_TESTING)
        set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
        set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
    endif()
endif()