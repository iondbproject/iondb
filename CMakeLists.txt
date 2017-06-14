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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -std=c++11 -Wno-vla -Wno-vla-extension -Wno-c99-extensions -Wno-writable-strings -Wno-write-strings")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -std=c++11 -Wno-missing-field-initializers")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -g -Wall -Wextra -pedantic -std=c99 -Wno-missing-field-initializers")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# Use cmake -DJENKINS_BUILD=ON <project_folder> to use XML format rather than the default.
if(JENKINS_BUILD)
    add_definitions(-DPLANCK_UNIT_OUTPUT_STYLE_XML)
endif()

# Add all of the CMakeLists.txt for the sub projects.
add_subdirectory(src/tests)
add_subdirectory(src/tests/unit/dictionary)

add_subdirectory(examples)

add_subdirectory(src/util/lfsr)

add_subdirectory(src/iinq)
add_subdirectory(src/dictionary/bpp_tree)
add_subdirectory(src/dictionary/flat_file)
add_subdirectory(src/dictionary/open_address_file_hash)
add_subdirectory(src/dictionary/open_address_hash)
add_subdirectory(src/dictionary/skip_list)

add_subdirectory(src/tests/unit/iinq)
add_subdirectory(src/tests/unit/dictionary/bpp_tree)
add_subdirectory(src/tests/unit/dictionary/flat_file)
add_subdirectory(src/tests/unit/dictionary/open_address_file_hash)
add_subdirectory(src/tests/unit/dictionary/open_address_hash)
add_subdirectory(src/tests/unit/dictionary/skip_list)

add_subdirectory(src/tests/behaviour/dictionary)
add_subdirectory(src/tests/behaviour/dictionary/flat_file)
add_subdirectory(src/tests/behaviour/dictionary/skip_list)
add_subdirectory(src/tests/behaviour/dictionary/bpp_tree)
add_subdirectory(src/tests/behaviour/dictionary/open_address_hash)
add_subdirectory(src/tests/behaviour/dictionary/open_address_file_hash)

add_subdirectory(src/cpp_wrapper)
add_subdirectory(src/tests/unit/cpp_wrapper)
add_subdirectory(src/tests/integration/cpp_wrapper)
