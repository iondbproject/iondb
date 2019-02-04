//
// Created by itsal on 2019-02-03.
//

#include "block_write_benchmarks.h"
#include <stdio.h>

int
main(
) {
    char *c = alloca(100);

    printf("Enter the path to the SD card (must end with /): ");
    gets(c);

    strcat(c, "blocks.dat");
    printf("\nYou entered: ");
    puts(c);

    ion_byte_t *buffer = alloca(512);
    for (int i = 0; i < 5; i++) {
        // Open a file for writing
        FILE *file = fopen(c, "w+b");

        if (NULL == file) {
            puts("\nUnable to open the file\n");
        } else {
            file_write_sequential(100000, file, buffer);
            file_read_sequential(100000, file, buffer);
            fclose(file);
        }
    }

    // In practice random reads are a lot slower so we do smaller numbers
    for (int i = 0; i < 5; i++) {
        // Open a file for writing
        FILE *file = fopen(c, "w+b");

        if (NULL == file) {
            puts("\nUnable to open the file\n");
        } else {
            file_write_sequential(10000, file, buffer);
            file_write_random(10000, 10000, file, buffer);
            file_read_random(10000, 10000, file, buffer);
            fclose(file);
        }
    }
    return 0;
}
