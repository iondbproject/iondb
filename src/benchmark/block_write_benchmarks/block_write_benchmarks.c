/*  */
/* Created by itsal on 2019-02-03. */
/*  */
#include "block_write_benchmarks.h"

int32_t
random_int(
        int32_t min,
        int32_t max
) {
#ifdef ARDUINO
    return random(min, max)
#else
    return min + rand() % (max - min);
#endif
}

void
randomize_buffer(
        unsigned char *buffer
) {
    uint64_t it;

    for (unsigned int i = 0; i < 512; i += 64) {
        it = *(buffer + i);
        it = ~(it >> 16);
    }

    int32_t first = *buffer;

    first = random_int(0, 1001);
}

void
file_read_random(
        int32_t count,
        int32_t file_blocks,
        FILE *file,
        ion_byte_t *buffer
) {
    printf("Reading %i random blocks,", count);
    fflush(stdout);
    if (NULL == file) {
        printf("Failed: Invalid file\n");
        return;
    }

    unsigned long start = ion_time();
    int32_t block;

    for (int32_t i = 0; i < count; i++) {
        block = random_int(0, file_blocks);

        if (0 != fseek(file, block * 512, SEEK_SET)) {
            printf("Unable to seek to block %i\n", block);
            return;
        }

        if (1 != fread(buffer, 512, 1, file)) {
            printf("Unable to read the file block %i\n", block);
            return;
        }
    }

    unsigned long time = ion_time() - start;

    printf("%lu\n", time);
}

void
file_read_sequential(
        int32_t count,
        FILE *file,
        ion_byte_t *buffer
) {
    printf("Reading %i sequential blocks,", count);
    fflush(stdout);
    if (NULL == file) {
        printf("Failed: Invalid file\n");
        return;
    }

    if (0 != fseek(file, 0, SEEK_SET)) {
        printf("Unable to seek to the start\n");
        return;
    }

    unsigned long start = ion_time();

    for (int32_t i = 0; i < count; i++) {
        if (1 != fread(buffer, 512, 1, file)) {
            printf("Unable to read the file block %i\n", i);
            return;
        }
    }

    unsigned long time = ion_time() - start;

    printf("%lu\n", time);
}

void
file_write_sequential(
        int32_t count,
        FILE *file,
        ion_byte_t *buffer
) {
    printf("Writing %i sequential blocks,", count);
    fflush(stdout);
    if (NULL == file) {
        printf("Failed: Invalid file\n");
        return;
    }

    unsigned long start = ion_time();

    for (int i = 0; i < count; i++) {
        randomize_buffer(buffer);

        if (1 != fwrite(buffer, 512, 1, file)) {
            printf("Unable to write file block %i\n", i);
            return;
        }
    }
    fflush(file);
    unsigned long time = ion_time() - start;
    printf("%lu\n", time);
}

void file_write_random(int32_t count, int32_t file_blocks, FILE *file, ion_byte_t *buffer) {
    printf("Writing %i random blocks,", count);
    fflush(stdout);

    if (NULL == file) {
        printf("Failed: Invalid file\n");
        return;
    }

    int32_t block = 0;
    unsigned long start = ion_time();
    for (int i = 0; i < count; i++) {
        randomize_buffer(buffer);
        block = random_int(0, file_blocks);
        if (0 != fseek(file, block * 512, SEEK_SET)) {
            printf("Unable to seek to file block %i\n", i);
        }
        if (1 != fwrite(buffer, 512, 1, file)) {
            printf("Unable to write file block %i\n", i);
            return;
        }
    }
    fflush(file);
    unsigned long time = ion_time() - start;
    printf("%lu\n", time);
}
