cmake_minimum_required(VERSION 3.5)
project(planck_unit)

set(SOURCE_FILES
    planck_unit.h
    planck_unit.c ../cpp_wrapper/BppTree.cpp ../cpp_wrapper/BppTree.h ../cpp_wrapper/FlatFile.cpp ../cpp_wrapper/FlatFile.h ../cpp_wrapper/LinearHash.cpp ../cpp_wrapper/LinearHash.cpp ../cpp_wrapper/LinearHash.cpp ../cpp_wrapper/LinearHash.h ../cpp_wrapper/OpenAddressFileHash.cpp ../cpp_wrapper/OpenAddressFileHash.h ../cpp_wrapper/OpenAddressHash.cpp ../cpp_wrapper/OpenAddressHash.h ../cpp_wrapper/SkipList.cpp ../cpp_wrapper/SkipList.h ../cpp_wrapper/CursorIterator.cpp ../cpp_wrapper/CursorIterator.h ../cpp_wrapper/CppWrapperTest.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

# Required on Unix OS family to be able to be linked into shared libraries.
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

project(cpp_wrapper)
add_executable(cpp_wrapper ../cpp_wrapper/CppWrapperTest.cpp)