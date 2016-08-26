/******************************************************************************/
/**
@file
@author		Wade Penson
@brief		Interface for the implementation of the external merge sort
			algorithm.
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

#if !defined(ION_EXTERNAL_MERGE_SORT_H_)
#define ION_EXTERNAL_MERGE_SORT_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "external_sort_types.h"
#include "../sort.h"
#include "../../../key_value/kv_system.h"

#if defined(ARDUINO)
#include "../../../serial/printf_redirect.h"
#include "../../../serial/serial_c_iface.h"
#include "../../../file/kv_stdio_intercept.h"
#include "../../../file/SD_stdio_c_iface.h"
#endif

typedef struct {

} ion_external_merge_sort_t;

/**
@brief		Initializes the external merge sort algorithm.

@param[in]	es
 				An initialized external sort structure.
@param[in]	cursor
 				An initialized external sort cursor structure.
@return		An error defined in @ref ion_err_t.
*/
ion_err_t
ion_external_merge_sort_init(
	ion_external_sort_t			*es,
	ion_external_sort_cursor_t	*cursor
);

/**
@brief

@param[in]	cursor
 				An initialized external sort cursor structure.
@param[out] output_value
 				A place in memory for the returned value from next.
@return		An error defined in @ref ion_err_t.
*/
ion_err_t
ion_external_merge_sort_next(
	ion_external_sort_cursor_t *cursor,
	void *output_value
);

#endif /* ION_EXTERNAL_MERGE_SORT_H_ */
