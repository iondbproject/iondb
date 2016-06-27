/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Source file for ram check utility function.
*/
/******************************************************************************/

#include "ram_util.h"

int
free_ram(
) {
	extern int	__heap_start, *__brkval;
	int			v;

	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
