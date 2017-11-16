cmake_minimum_required(VERSION 3.5)
project(StaticExample)

set(SOURCE_FILES
        DogExample.cpp)

add_executable(${PROJECT_NAME}          ${SOURCE_FILES} DogExample.cpp)

target_link_libraries(${PROJECT_NAME}   cpp_wrapper)

# Use cmake -DCOVERAGE_TESTING=ON to include coverage testing information.
if (CMAKE_COMPILER_IS_GNUCC AND COVERAGE_TESTING)
    set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
endif()