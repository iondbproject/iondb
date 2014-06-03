/*
 * System.h
 *
 *  Created on: May 20, 2014
 *      Author: workstation
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#define USING_ECLIPSE 1
#define DEBUG 1
#define IS_EQUAL 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum status
{
	status_ok,
	status_item_not_found,
	status_duplicate_key,
};

typedef char status_t;

enum error
{
	err_ok = 0,
	err_item_not_found,
	err_duplicate_key,
	err_max_capacity,
	err_write_concern,
	err_colllection_destruction_error,
};


typedef char err_t;


#endif /* SYSTEM_H_ */
