/******************************************************************************/
/**
@file
@author		Scott Fazackerley
@brief		Provides a printf implementaton for C code running from inside an
			Arduino Sketch.

			IMPORTANT: This must be included after stdio.h to function properly.
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

#if !defined(SERIAL_C_IFACE_H_)
#define SERIAL_C_IFACE_H_

#include <Arduino.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
@brief		Setting this value to 1 will use an inline replacement for printf
			which reduces call stack requirements (minimal) but increase code
			footprint considerably for each use of printf. You decide if you
			want speed vs. minimal code space.
*/
#define USE_INLINEPRINTF 0

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
);

/* DO NOT CHANGE INCLUSION ORDER. */
#if USE_INLINE_PRINTF == 1
#include "printf_redirect.h"
#else
#define printf(format, ...) serial_printf_c(format, ## __VA_ARGS__)
#endif

/**
@brief	  A print function wrapping Arduino's serial stream.
@param		buffer
				Pointer to the character array / buffer to print.
@return		The number of characters outputted.
*/
extern int
serial_print(
	const char *buffer
);

/**
@brief		Initializes serial port 0 for communications.
@details	By default the port is set up at N-8-1.
@param	  baud_rate
				The buad rate to be used.
*/
void
serial_init(
	int baud_rate
);

/**
@brief		Closes the communication port so that the pins can be used as
			general I/O.
*/
void
serial_close(
);

#if defined(__cplusplus)
}
#endif
#endif /* SERIAL_C_IFACE_H_ */
