#include <SPI.h>
#include <SD.h>
#include "linear_hash_benchmarks.h"

void
setup(
) {
    SPI.begin();
    SD.begin(SD_CS_PIN);
    Serial.begin(BAUD_RATE);
    while (!Serial){
        ;
    }
    Serial.print("Starting Benchmarks\n");
    run_benchmarks();
}

void
loop(
) {}
