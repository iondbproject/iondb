//
// Created by itsal on 2019-02-03.
//
#include <stdio.h>
#include "../../file/sd_stdio_c_iface.h"
#include "../../file/kv_stdio_intercept.h"
#include "./../../tests/planck-unit/src/ion_time/ion_time.h"

#ifndef IONDB_BLOCK_WRITE_BENCHMARKS_H
#define IONDB_BLOCK_WRITE_BENCHMARKS_H


/**
 * Reads [count] random blocks from a file and prints out the result
 * @param count The number of random blocks to read
 * @param file_blocks The total number of blocks in the file
 * @param file The file to read from.
 */
void
file_read_random(int32_t count, int32_t file_blocks, FILE *file, ion_byte_t *buffer);

/**
 * Reads [count] blocks from a file sequentially
 * @param count The number of blocks to read
 * @param file The file to read from
 */
void file_read_sequential(int32_t count, FILE *file, ion_byte_t *buffer);

void file_write_sequential(int32_t count, FILE *file, ion_byte_t *buffer);

void
file_write_random(
        int32_t count,
        int32_t file_blocks,
        FILE *file,
        ion_byte_t *buffer
);

#endif //IONDB_BLOCK_WRITE_BENCHMARKS_H
