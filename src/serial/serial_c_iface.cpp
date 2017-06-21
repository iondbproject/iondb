/******************************************************************************/
/**
@file		serial_c_iface.cpp
@author		Scott Fazackerley
@brief		Wraps the Arduino Serial object and provides a simple printf
			implementation for C.
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

#include "serial_c_iface.h"

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

void
serial_init(
	int baud_rate
) {
	Serial.begin(baud_rate);
}

void
serial_close(
) {
	Serial.end();
}
