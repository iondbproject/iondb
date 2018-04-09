cmake_minimum_required(VERSION 3.5)
project(test_iinq_rewrite)

set(SOURCE_FILES
        test_iinq_rewrite.h
        test_iinq_rewrite.c)

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})
    set(${PROJECT_NAME}_PORT        ${PORT})
    set(${PROJECT_NAME}_SERIAL      ${SERIAL})

    set(${PROJECT_NAME}_SKETCH      iinq_rewrite.ino)
    set(${PROJECT_NAME}_SRCS        ${SOURCE_FILES})
    set(${PROJECT_NAME}_LIBS        planck_unit iinq iinq_rewrite external_sort flat_file skip_list open_address_file_hash open_address_hash linear_hash)

    generate_arduino_firmware(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME}          run_iinq_rewrite.c ${SOURCE_FILES})

    target_link_libraries(${PROJECT_NAME}   planck_unit iinq iinq_rewrite flat_file external_sort flat_file skip_list open_address_file_hash open_address_hash linear_hash)

    # Use cmake -DCOVERAGE_TESTING=ON to include coverage testing information.
    if (CMAKE_COMPILER_IS_GNUCC AND COVERAGE_TESTING)
        set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
        set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
    endif()
endif()