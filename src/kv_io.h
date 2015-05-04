/*
 * io.h
 *
 *  Created on: May 23, 2014
 *      Author: workstation
 */

#ifndef IO_H_
#define IO_H_

#include <stdarg.h>
#include <stdio.h>

#include "kv_system.h"

/**
 * @brief Allows for output in a tidy format.
 *
 * @details Used as an alternate printf to deal with device specific issues
 *
 * @param format
 * @return
 */
int io_printf( const char * format, ... );

#endif /* IO_H_ */
