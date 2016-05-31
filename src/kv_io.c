/******************************************************************************/
/**
@file		kv_io.c
@author		Scott Fazackerley
@brief		Key-Value I/O functionality.
*/
/******************************************************************************/

#include "kv_io.h"

int
io_printf(
	const char *format,
	...
) {

	int 	result;
	va_list args;

	va_start(args, format);
	result = vprintf(format, args);
	va_end(args);

#if defined(USING_ECLIPSE)
	fflush(stdout);
#endif

	return result;
}