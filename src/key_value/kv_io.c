/******************************************************************************/
/**
@author		Scott Fazackerley
@brief		Key-Value I/O functionality.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
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

#include "kv_io.h"

int
io_printf(
	const char *format,
	...
) {
	int		result;
	va_list args;

	va_start(args, format);
	result = vprintf(format, args);
	va_end(args);

#if defined(USING_ECLIPSE)
	fflush(stdout);
#endif

	return result;
}
