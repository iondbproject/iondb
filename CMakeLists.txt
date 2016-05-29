cmake_minimum_required(VERSION 3.5)

# Use cmake -DUseArduino=ON <project_folder> to compile to Arduino.
if(UseArduino)
    message("Linking against Arduino...")

    set(CMAKE_TOOLCHAIN_FILE    ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
    set(PROJECT_NAME            IonDB)
    project(IonDB               C CXX)

    # User options
    set(BOARD       mega)
    set(PROCESSOR   atmega2560)
    set(PORT        /dev/cu.usbmodem1411)
    set(BAUD_RATE   115200)
    set(MANUAL      false)

    add_definitions(-DBAUD_RATE=${BAUD_RATE})

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
endif()

# Add all of the CMakeLists.txt for the sub projects.
add_subdirectory(src/tests)
add_subdirectory(src/tests/unit/dictionary)

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