cmake_minimum_required(VERSION 3.5)
project(planck_unit)

set(SOURCE_FILES
    planckunit/src/planck_unit.h
    planckunit/src/planck_unit.c)

if(USE_ARDUINO)
    set(${PROJECT_NAME}_BOARD       ${BOARD})
    set(${PROJECT_NAME}_PROCESSOR   ${PROCESSOR})
    set(${PROJECT_NAME}_MANUAL      ${MANUAL})

    set(${PROJECT_NAME}_SRCS
        ${SOURCE_FILES}
        ../serial/serial_c_iface.h
        ../serial/serial_c_iface.cpp
        ../serial/printf_redirect.h)

    generate_arduino_library(${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

    # Required on Unix OS family to be able to be linked into shared libraries.
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# PlanckUnit option: cmake -DJENKINS_BUILD=ON <project_folder> to use XML format rather than the default.
if(JENKINS_BUILD)
    add_definitions(-DPLANCK_UNIT_OUTPUT_STYLE_XML)
endif()

# Use cmake -DCOVERAGE_TESTING=ON to include coverage testing information.
option(COVERAGE_TESTING "Enable profiling and coverage report" OFF)
if (CMAKE_COMPILER_IS_GNUCC AND COVERAGE_TESTING)
    set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
endif()