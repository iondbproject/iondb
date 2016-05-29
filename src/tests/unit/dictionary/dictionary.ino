#include <SPI.h>
#include <Arduino.h>
#include "test_dictionary.h"

void
setup(
)
{
    SPI.begin();
    Serial.begin(BAUD_RATE);
    runalltests_dictionary();
}

void
loop(
)
{

}