#include <stdio.h>

#include "benchmark.h"
//#include "windows.h"

int main(void) {
    benchmark_start();
    time_counter = time(NULL);
    printf("Wait 5 seconds...\n");
    Sleep(5000);
    benchmark_stop();

    return 0;
}