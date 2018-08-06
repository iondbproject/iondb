cmake_minimum_required(VERSION 3.5)
project(iinq_new)

set(SOURCE_FILES
        iinq_user.c
        iinq_user.h
        iinq_functions.c
        iinq_functions.h
        iinq_user_functions.c
        iinq_user_functions.h
        ../../dictionary/dictionary_types.h
        ../../dictionary/dictionary.c
        ../../dictionary/dictionary.h
        )

    add_executable(${PROJECT_NAME}          ${SOURCE_FILES})

    target_link_libraries(${PROJECT_NAME} bpp_tree flat_file iinq linear_hash open_address_hash open_address_file_hash skip_list)
    # Use cmake -DCOVERAGE_TESTING=ON to include coverage testing information.
    if (CMAKE_COMPILER_IS_GNUCC AND COVERAGE_TESTING)
        set(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
        set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
    endif()