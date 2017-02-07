#include <SPI.h>
#include <SD.h>
#include "test_linear_hash.h"
#include "test_linear_hash_dictionary_handler.h"

void
setup(
) {
	SPI.begin();
	SD.begin(SD_CS_PIN);
	Serial.begin(BAUD_RATE);
	runalltests_linear_hash();
	runalltests_linear_hash_handler();
}

void
loop(
) {}
