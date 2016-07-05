#include <Arduino.h>
#include "test_cpp_wrapper.h"

void
setup(
)
{
    Serial.begin(BAUD_RATE);
    runalltests_cpp_wrapper();
}

void
loop(
)
{

}