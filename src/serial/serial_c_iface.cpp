/******************************************************************************/
/**
@file
@author		Scott Fazackerley
@brief		Wraps the Arduino Serial object and provides a simple printf
			implementation for C.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
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

/**
@brief		A version of printf that limits the number of characters
			printed per call.
@details	When additional characters are requested for printing, the
			print fails.
@param		format
				 The string that contains the text to be written to serial.
@param		...
				Variable arguments of data used by the format.
@return	 The number of characters written. Returns a negative value on
			failure.
*/
int
serial_printf_c(
	const char *format,
	...
) {
	va_list args;

	va_start(args, format);

	/* +1 for the null terminator \0 at the end */
	int		bufsize = vsnprintf(NULL, 0, format, args) + 1;
	char	buf[bufsize];

	va_end(args);

	va_start(args, format);
	vsnprintf(buf, bufsize, format, args);
	va_end(args);

	return serial_print(buf);
}

int
serial_print(
	const char *buffer
) {
	int num;

	num = Serial.print(buffer);
#if DEBUG
	Serial.flush();
#endif
	return num;
}

/**
@brief		Initializes serial port 0 for communications.
@details	By default the port is set up at N-8-1.
@param	  baud_rate
				The buad rate to be used.
*/
void
serial_init(
	int baud_rate
) {
	Serial.begin(baud_rate);
}

/**
@brief		Closes the communication port so that the pins can be used as
			general I/O.
*/
void
serial_close(
) {
	Serial.end();
}
