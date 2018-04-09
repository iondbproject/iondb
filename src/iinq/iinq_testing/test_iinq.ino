#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>
#include "test_iinq_device.h"

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
	int i;
	int num_records = 30;

    for (i = 0; i < 3; i++) {
        Serial.println(freeRam());
        run_all_tests_iinq_device(num_records);
        num_records = num_records-10;
        Serial.println(freeRam());

        pinMode(13, OUTPUT);

        for (int j = 0 ; j < EEPROM.length() ; j++) {
            EEPROM.write(j, 0);
        }

        // turn the LED on when we're done
        digitalWrite(13, HIGH);
    }
}

void
loop(
) {}
