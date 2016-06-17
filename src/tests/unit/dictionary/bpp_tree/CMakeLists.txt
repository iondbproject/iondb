cmake_minimum_required(VERSION 3.5)
project(test_bpp_tree)

set(SOURCE_FILES
    test_bpp_tree_handler.h
    test_bpp_tree_handler.c
    ../generic_dictionary_test.h
    ../generic_dictionary_test.c)

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})
    set(${PROJECT_NAME}_PORT        ${PORT})
    set(${PROJECT_NAME}_SERIAL      ${SERIAL})

    set(${PROJECT_NAME}_SKETCH      bpp_tree.ino)
    set(${PROJECT_NAME}_SRCS        ${SOURCE_FILES})
    set(${PROJECT_NAME}_LIBS        planck_unit bpp_tree)

    generate_arduino_firmware(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME}          run_bpp_tree.c ${SOURCE_FILES})

    target_link_libraries(${PROJECT_NAME}   planck_unit bpp_tree)
endif()

# Use cmake -DCOVERAGE_TESTING=ON to include coverage testing information.
option(COVERAGE_TESTING "Enable profiling and coverage report" OFF)
if (CMAKE_COMPILER_IS_GNUCC AND COVERAGE_TESTING)
    set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
endif()