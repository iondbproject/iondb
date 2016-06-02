/******************************************************************************/
/**
@file
@author		Scott Fazackerley
@brief		Key-Value I/O functionality.
*/
/******************************************************************************/


#if !defined(KV_IO_H_)
#define KV_IO_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "kv_system.h"

/* Only on PC */
#if !defined(ARDUINO)
#define fremove(x) remove(x)
#define frewind(x) rewind(x)
#endif

/**
@brief		The function outputs a string in a tidy format.
@details	Used as an alternate printf to deal with device specific issues.
@param		format
				Format string to specify print output.
@return		If successful, the total number of characters written is returned.
 			Otherwise, a negative number is returned.
*/
int
io_printf(
	const char *format,
	...
);

#endif /* KV_IO_H_ */