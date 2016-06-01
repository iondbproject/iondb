#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "test_bpp_tree_handler.h"

void
setup(
)
{
    SPI.begin();
    Serial.begin(BAUD_RATE);
    run_all_tests_bpptreehandler();
}

void
loop(
)
{

}