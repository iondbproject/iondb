cmake_minimum_required(VERSION 3.5)

# Use cmake -DUSE_ARDUINO=ON <project_folder> to compile to Arduino.
if(USE_ARDUINO)
    message("Linking against Arduino...")
    set(CMAKE_TOOLCHAIN_FILE    ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
    set(PROJECT_NAME            IonDB)
    project(IonDB               C CXX)

    # User options
    set(BOARD ${BOARD})

    if(NOT PROCESSOR)
        set(PROCESSOR)
    endif()

    set(PORT ${PORT})

    if(NOT BAUD_RATE)
        set(BAUD_RATE 9600)
    endif()

    if(NOT MANUAL)
        set(MANUAL FALSE)
    endif()

    if(NOT FORMATTED_SD_CARD_CS)
        set(FORMATTED_SD_CARD_CS 4)
    endif()

    message(${BAUD_RATE})
    add_definitions("-D BAUD_RATE=${BAUD_RATE} -D SD_CS_PIN=${FORMATTED_SD_CARD_CS}")

    if(WIN32)
        set(SERIAL putty -serial ${PORT} -sercfg ${BAUD_RATE})
    else()
        set(SERIAL screen ${PORT} ${BAUD_RATE})
    endif()

    print_board_list()
    print_programmer_list()
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -g")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# Use cmake -DCOVERAGE_TESTING=ON to include coverage testing information.
option(COVERAGE_TESTING "Enable profiling and coverage report" OFF)
if (CMAKE_COMPILER_IS_GNUCC AND COVERAGE_TESTING)
    set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
    set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
endif()

# Add all of the CMakeLists.txt for the sub projects.
add_subdirectory(src/tests)
add_subdirectory(src/tests/unit/dictionary)

add_subdirectory(src/examples)

add_subdirectory(src/util/lfsr)

add_subdirectory(src/util/id_encoder)
add_subdirectory(src/tests/unit/dictionary/id_encoder)

add_subdirectory(src/dictionary/bpp_tree)
add_subdirectory(src/dictionary/flat_file)
add_subdirectory(src/dictionary/linear_hash)
add_subdirectory(src/dictionary/open_address_file_hash)
add_subdirectory(src/dictionary/open_address_hash)
add_subdirectory(src/dictionary/skip_list)

add_subdirectory(src/tests/unit/dictionary/bpp_tree)
add_subdirectory(src/tests/unit/dictionary/flat_file)
add_subdirectory(src/tests/unit/dictionary/linear_hash)
add_subdirectory(src/tests/unit/dictionary/open_address_file_hash)
add_subdirectory(src/tests/unit/dictionary/open_address_hash)
add_subdirectory(src/tests/unit/dictionary/skip_list)
