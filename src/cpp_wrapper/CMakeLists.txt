cmake_minimum_required(VERSION 3.5)
project(cpp_wrapper)

set(SOURCE_FILES
    CursorIterator.h
    CursorIterator.cpp
    CppDictionary.h
    CppDictionary.cpp
    BppTree.h
    BppTree.cpp
    FlatFile.h
    FlatFile.cpp
    LinearHash.h
    LinearHash.cpp
    OpenAddressFileHash.h
    OpenAddressFileHash.cpp
    OpenAddressHash.h
    OpenAddressHash.cpp
    SkipList.h
    SkipList.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_link_libraries(
    ${PROJECT_NAME}
    bpp_tree
    flat_file
    linear_hash
    open_address_file_hash
    open_address_hash
    skip_list)