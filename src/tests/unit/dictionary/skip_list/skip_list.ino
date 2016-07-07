#include <Arduino.h>
#include "test_skip_list.h"
#include "test_skip_list_handler.h"

void
setup(
) {
	Serial.begin(BAUD_RATE);
	runalltests_skiplist();
	runalltests_skiplist_handler();
}

void
loop(
) {}
