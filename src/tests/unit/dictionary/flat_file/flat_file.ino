#include <SPI.h>
#include <SD.h>
#include "test_flat_file.h"
#include "test_flat_file_dictionary_handler.h"

void
setup(
) {
	SPI.begin();
	SD.begin(SD_CS_PIN);
	Serial.begin(BAUD_RATE);
	runalltests_flat_file();
	runalltests_flat_file_handler();
}

void
loop(
) {}
