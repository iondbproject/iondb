/******************************************************************************/
/**
@file		sketch.ino
@author		Scott Fazackerley
@brief		Sample code that calls c library to demonstrate the use of printf
			overloading for the Arduino
*/
/******************************************************************************/

#include "test_printf.h"

void setup()
{
	Serial.begin(9600);
	Serial.println("This is the start of the example");
	test_print();
	Serial.end();
}

void loop()
{
}
