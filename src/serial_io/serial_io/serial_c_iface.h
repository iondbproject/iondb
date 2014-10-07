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

#include "printf_redirect.h"

/**
@brief 		Wrapper to Serial stream
@param		buffer
				pointer to buffer to print
@return		The number of characters outputted
*/
int
serial_print(
	const char 	*buffer
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
