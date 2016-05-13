cmake_minimum_required(VERSION 3.5)

option(UseArduino "Link against the Arduino library" OFF)

#SET(UseArduino TRUE)

# Use cmake -DArduino=ON MyProjectFolder to enable.
if(UseArduino)
    message("Linking against Arduino...")
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
    message(STATUS "CMAKE_TOOLCHAIN_FILE='${CMAKE_TOOLCHAIN_FILE}'")

    set(BOARD uno)
    set(MANUAL false)
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")
endif()

set(PROJECT_NAME IonDB)
project(IonDB C CXX)

#print_board_list()
#print_programmer_list()

#add_subdirectory(src/tests)
#add_subdirectory(src/tests/unit/dictionary)
#
#add_subdirectory(src/util/id_encoder)
#add_subdirectory(src/tests/unit/dictionary/id_encoder)

add_subdirectory(src/dictionary/bpp_tree)
#add_subdirectory(src/dictionary/flat_file)
#add_subdirectory(src/dictionary/linear_hash)
#add_subdirectory(src/dictionary/open_address_file_hash)
#add_subdirectory(src/dictionary/open_address_hash)
#add_subdirectory(src/dictionary/skip_list)
#
add_subdirectory(src/tests/unit/dictionary/bpp_tree)
#add_subdirectory(src/tests/unit/dictionary/flat_file)
#add_subdirectory(src/tests/unit/dictionary/linear_hash)
#add_subdirectory(src/tests/unit/dictionary/open_address_file_hash)
#add_subdirectory(src/tests/unit/dictionary/open_address_hash)
#add_subdirectory(src/tests/unit/dictionary/skip_list)