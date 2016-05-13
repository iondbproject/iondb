/**
* benchmark.c
* Author: Eric
* Date: Feb. 4th 2015
*/

#include "benchmark.h"

void
benchmark_start(
    void
)
{
    ram_counter = free_ram();
    time_counter = millis();
}

void
benchmark_stop(
    void
)
{
    unsigned int ram_used = ram_counter - free_ram() - 4; //Magic offset - compensates for benchmark requiring 4 bytes to work
    unsigned int time_elapsed = millis() - time_counter;
    printf("%db RAM use in %dms.\n", ram_used, time_elapsed);
}