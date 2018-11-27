#include <SPI.h>
#include <SD.h>
#include "test_linear_hash.h"

void
setup(
) {
	SPI.begin();
	SD.begin(SD_CS_PIN);
	Serial.begin(BAUD_RATE);
	Serial.print(F("Starting tests\n"));
	runalltests_linear_hash();
}

void
loop(
) {}
