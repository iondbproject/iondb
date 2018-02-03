#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "test_cpp_wrapper2.h"

void
setup(
) {
	SPI.begin();
	SD.begin(SD_CS_PIN);
	Serial.begin(BAUD_RATE);
	runalltests_cpp_wrapper2();
}

void
loop(
) {}
