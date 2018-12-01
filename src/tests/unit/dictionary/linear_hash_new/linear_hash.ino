#include <SPI.h>
#include <SD.h>
#include "test_linear_hash.h"

void
setup(
) {
	SPI.begin();
	SD.begin(SD_CS_PIN);
	Serial.begin(BAUD_RATE);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB
	}
	Serial.print(F("Starting tests\n"));
	runalltests_linear_hash();
}

void
loop(
) {}
