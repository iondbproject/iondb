#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "test_cursor_behaviour_open_address_file_hash.h"

void
setup(
) {
	SPI.begin();
	SD.begin(SD_CS_PIN);
	Serial.begin(BAUD_RATE);
	runalltests_cursor_behaviour_open_address_file_hash();
}

void
loop(
) {}
