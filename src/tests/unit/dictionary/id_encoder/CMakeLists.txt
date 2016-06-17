cmake_minimum_required(VERSION 3.5)
project(test_id_encoder)

set(SOURCE_FILES
    test_file_encoder.h
    test_file_encoder.c)

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})
    set(${PROJECT_NAME}_PORT        ${PORT})
    set(${PROJECT_NAME}_SERIAL      ${SERIAL})

    set(${PROJECT_NAME}_SKETCH      id_encoder.ino)
    set(${PROJECT_NAME}_SRCS        ${SOURCE_FILES})
    set(${PROJECT_NAME}_LIBS        planck_unit id_encoder)

    generate_arduino_firmware(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME}          ${SOURCE_FILES} run_file_encoder.c)

    target_link_libraries(${PROJECT_NAME}   planck_unit id_encoder)
endif()

# Use cmake -DCOVERAGE_TESTING=ON to include coverage testing information.
option(COVERAGE_TESTING "Enable profiling and coverage report" OFF)
if (CMAKE_COMPILER_IS_GNUCC AND COVERAGE_TESTING)
    set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
endif()