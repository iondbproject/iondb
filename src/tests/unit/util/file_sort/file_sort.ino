#include <SPI.h>
#include <SD.h>
#include "test_file_sort.h"

void
setup(
) {
	SPI.begin();
	SD.begin(SD_CS_PIN);
	Serial.begin(BAUD_RATE);
	runalltests_file_sort();
}

void
loop(
) {}
