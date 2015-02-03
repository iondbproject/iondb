/******************************************************************************/
/**
@file		serial_c_iface.h
@author		Scott Fazackerley
@brief		Proivdes a printf implementaton for c code running from inside an
			Arduino Sketch.

			IMPORTANT: This must be included after stdio.h to function properly
*/
/******************************************************************************/

#ifndef SERIAL_C_IFACE_H_
#define SERIAL_C_IFACE_H_

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USE_INLINEPRINTF 0	/** <setting this value to 1 will use an inline replacement for printf
								which reduces call stack requirements (minimal) but increased code
								footprint considerably for each use of printf.  You decide if you
								want speed vs. minimal code space */



/**
 @brief		Overload of printf for arduino to allow a limited use printf
 @param 	format
 	 	 	 	 The string that contains the text to be written to serial.
 @return 	The number of characters written.  Returns a negative value on failure.
 */
int
serial_printf_c(
	const char 	*format,
	...
);

/** DO NOT CHANGE INCLUSION ORDER */
#if USE_INLINE_PRINTF == 1
	#include "printf_redirect.h"
#else
	#define printf(format, ...) 	serial_printf_c(format, ##__VA_ARGS__)
#endif

/**
@brief 		Wrapper to Serial stream
@param		buffer
				pointer to buffer to print
@return		The number of characters outputted
*/
extern int
serial_print(
	const char 		*buffer
);


/**
@brief		Initializes serial port 0 for communications.  By default
 	 	 	the port is set up at N-8-1.
@param 		baud_rate
				The buad rate to be used.
*/
void
serial_init(
	int			baud_rate
);

/**
@brief		Closes the comm port so that the pins can be used as general i/o.
 */
void
serial_close(
);

#ifdef __cplusplus
}
#endif
#endif /* SERIAL_C_IFACE_H_ */
