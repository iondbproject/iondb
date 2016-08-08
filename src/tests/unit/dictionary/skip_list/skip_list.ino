#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "test_skip_list.h"
#include "test_skip_list_handler.h"

void
setup(
) {
    SPI.begin();
    SD.begin(SD_CS_PIN);
	Serial.begin(BAUD_RATE);
	runalltests_skiplist();
	runalltests_skiplist_handler();
}

void
loop(
) {}
