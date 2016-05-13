cmake_minimum_required(VERSION 3.5)
project(test_id_encoder)

set(SOURCE_FILES
    run_file_encoder.c
    test_file_encoder.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} planck_unit id_encoder)