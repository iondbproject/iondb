#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "test_open_address_file_hash.h"
#include "test_open_address_file_hash_dictionary_handler.h"

void
setup(
)
{
    SPI.begin();
    SD.begin(SD_CS_PIN);
    Serial.begin(BAUD_RATE);
    runalltests_open_address_file_hash();
    runalltests_open_address_file_hash_handler();
}

void
loop(
)
{

}