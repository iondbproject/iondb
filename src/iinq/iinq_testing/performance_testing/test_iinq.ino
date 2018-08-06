#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>
#include "test_iinq_device.h"

void cleanup() {
	fdeleteall();
	fremove("0.inq");
	fremove(ION_MASTER_TABLE_FILENAME);
	fremove("1.ffs");
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void
setup(
) {
	SPI.begin();
	SD.begin(SD_CS_PIN);
	Serial.begin(BAUD_RATE);
	cleanup();
	run_all_tests_iinq_device();
}

void
loop(
) {}
