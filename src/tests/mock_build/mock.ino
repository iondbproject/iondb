#include <Arduino.h>
#include "test_mock.h"

void
setup(
) {
	Serial.begin(BAUD_RATE);
	runalltests_mock();
}

void
loop(
) {}
