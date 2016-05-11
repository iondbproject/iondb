cmake_minimum_required(VERSION 3.5)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

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