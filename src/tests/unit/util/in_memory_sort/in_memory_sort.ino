#include <Arduino.h>
#include "test_in_memory_sort.h"

void
setup(
) {
	Serial.begin(BAUD_RATE);
	runalltests_in_memory_sort();
}

void
loop(
) {}
