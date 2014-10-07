/******************************************************************************/
/**
@file		serial_c_iface.cpp
@author		Scott Fazackerley
@brief		Wraps the Arduino Serial object and provides a simple printf
 	 	 	implementation for c called from an Arduino sketch
*/
/******************************************************************************/

#include "serial_c_iface.h"

int
serial_print(
	const char 	*buffer
)
{ 
    Serial.print(buffer);
}

void
serial_init(
	int			baud_rate
)
{
	Serial.begin(baud_rate);
}


void
serial_close(
)
{
	Serial.end();
}

