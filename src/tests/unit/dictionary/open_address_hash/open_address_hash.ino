#include <Arduino.h>
#include "test_open_address_hash.h"
#include "test_open_address_hash_dictionary_handler.h"

void
setup(
)
{
    Serial.begin(9600);
    Serial.println("test");
    runalltests_open_address_hash();
    runalltests_open_address_hash_handler();
}

void
loop(
)
{

}