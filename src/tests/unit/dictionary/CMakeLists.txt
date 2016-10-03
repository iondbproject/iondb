cmake_minimum_required(VERSION 3.5)
project(test_dictionary)

set(SOURCE_FILES
    ../../../dictionary/ion_master_table.h
    ../../../dictionary/ion_master_table.c
    test_dictionary.h
    generic_dictionary_test.h
    generic_dictionary_test.c)

if(USE_ARDUINO)
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

    set(${PROJECT_NAME}_LIBS        planck_unit skip_list flat_file)

    generate_arduino_firmware(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME}          ${SOURCE_FILES} run_dictionary.c)

    target_link_libraries(${PROJECT_NAME}   planck_unit skip_list flat_file)

    # Use cmake -DCOVERAGE_TESTING=ON to include coverage testing information.
    if (CMAKE_COMPILER_IS_GNUCC AND COVERAGE_TESTING)
        set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
        set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
    endif()
endif()