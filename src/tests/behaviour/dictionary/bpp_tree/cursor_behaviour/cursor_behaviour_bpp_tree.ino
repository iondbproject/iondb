#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "test_cursor_behaviour_bpp_tree.h"

void
setup(
) {
	SPI.begin();
	SD.begin(SD_CS_PIN);
	Serial.begin(BAUD_RATE);
	runalltests_cursor_behaviour_bpp_tree();
}

void
loop(
) {}
