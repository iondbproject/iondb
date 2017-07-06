/******************************************************************************/
/**
@file		serial_c_iface.h
@author		Scott Fazackerley
@brief		Provides a printf implementaton for C code running from inside an
			Arduino Sketch.
@details	IMPORTANT: This must be included after stdio.h to function properly.
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
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
#define ION_USE_INLINEPRINTF 0

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
