/******************************************************************************/
/**
@file		serial_c_iface.cpp
@author		Scott Fazackerley
@brief		Wraps the Arduino Serial object and provides a simple printf
		implementation for c called from an Arduino sketch
@details	This file originally comes from the IonDB project.
@copyright      Copyright 2014 Scott Fazackerley
@license        Licensed under the Apache License, Version 2.0 (the "License");
                you may not use this file except in compliance with the License.
                You may obtain a copy of the License at
                        http://www.apache.org/licenses/LICENSE-2.0

@par
                Unless required by applicable law or agreed to in writing,
                software distributed under the License is distributed on an
                "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
                either express or implied. See the License for the specific
                language governing permissions and limitations under the
                License.
*/
/******************************************************************************/

#include "serial_c_iface.h"

int
serial_printf_c(
	const char		*format,
	...
)
{
	char buf[128];							// resulting string limited to 128 chars
    va_list args;
    va_start (args, format);
    vsnprintf(buf, 128, format, args);
    va_end(args);
    return serial_print(buf);
}

int
serial_print(
	const char	*buffer
)
{ 
    Serial.print(buffer);
#if DEBUG
    Serial.flush();
#endif
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

