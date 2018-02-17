cmake_minimum_required(VERSION 3.5)
project(test_cpp_wrapper4)

set(SOURCE_FILES
    test_cpp_wrapper4.cpp
    test_cpp_wrapper4.h
    ../test3/test_cpp_wrapper3.cpp
    ../test3/test_cpp_wrapper3.h
    ../test1/test_cpp_wrapper1.h
    ../test1/test_cpp_wrapper1.cpp
    ../test2/test_cpp_wrapper2.h
    ../test2/test_cpp_wrapper2.cpp
    ../../../../cpp_wrapper/MasterTable.h
    ../../../../dictionary/ion_master_table.h
    ../../../../dictionary/ion_master_table.c)


if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})
    set(${PROJECT_NAME}_PORT        ${PORT})
    set(${PROJECT_NAME}_SERIAL      ${SERIAL})

    set(${PROJECT_NAME}_SKETCH      cpp_wrapper4.ino)
    set(${PROJECT_NAME}_SRCS        ${SOURCE_FILES})
    set(${PROJECT_NAME}_LIBS        planck_unit cpp_wrapper)

    generate_arduino_firmware(${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME}          ${SOURCE_FILES} run_cpp_wrapper4.cpp)

    target_link_libraries(${PROJECT_NAME}   planck_unit cpp_wrapper)

    # Use cmake -DCOVERAGE_TESTING=ON to include coverage testing information.
    if (CMAKE_COMPILER_IS_GNUCC AND COVERAGE_TESTING)
        set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
        set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
    endif()
endif()

