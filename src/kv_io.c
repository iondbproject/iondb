/*
 * io.c
 *
 *  Created on: May 22, 2014
 *      Author: Scott Fazackerley
 */

#include "kv_io.h"

int io_printf( const char * format, ... )
{

	int result;

	va_list args;
	va_start(args, format);
	result = vprintf(format, args);
	va_end( args );

#ifdef USING_ECLIPSE
	fflush (stdout);
#endif

	return result;
}