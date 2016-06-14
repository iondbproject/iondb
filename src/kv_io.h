/******************************************************************************/
/**
@file
@author		Scott Fazackerley
@brief		Key-Value Input/Output functionality.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see @ref AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
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
#define fremove(x)	remove(x)
#define frewind(x)	rewind(x)
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

