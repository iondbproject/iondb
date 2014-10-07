/******************************************************************************/
/**
@file		test_printf.h
@author		Scott Fazackerley
@brief		Sample c code with printf overloaded for use with Arduino
*/
/******************************************************************************/
#ifndef TEST_PRINTF_H_
#define TEST_PRINTF_H_

#include <stdio.h>
#include "serial_io/serial_c_iface.h"		/**< Include the c to c++ wrapper for Serial */


#ifdef __cplusplus
extern "C" {
#endif

/**
@brief		Test of the printf overload for Arduino
*/
void
test_print();

#ifdef __cplusplus
}
#endif

#endif /* TEST_PRINTF_H_ */
