/******************************************************************************/
/**
@file
@author		Wade Penson
@brief		Interface for the implementation of the flash minsort algorithm.
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

#if !defined(ION_FLASH_MIN_SORT_H_)
#define ION_FLASH_MIN_SORT_H_

#include <stdio.h>
#include <stdint.h>
#include "external_sort_types.h"
#include "../sort.h"
#include "../../../key_value/kv_system.h"

typedef struct {
	uint32_t	num_pages_per_region;
	uint32_t	num_regions;
	long		next_index;
} ion_flash_min_sort_t;

/**
@brief		Initializes the minsort algorithm. This calculates the number of regions
			and size of the minimum index based on the number of values. It does a
			one pass scan through the file to initialize the minimum index.

@param[in]	es		An initialized external sort structure.
@param[in]	cursor	An initialized external sort cursor structure.
@return		An error defined in @ref ion_err_t.
*/
ion_err_t
ion_flash_min_sort_init(
	ion_external_sort_t			*es,
	ion_external_sort_cursor_t	*cursor
);

/**
@brief

@param[in]	cursor	An initialized external sort cursor structure.
@return		An error defined in @ref ion_err_t.
*/
ion_err_t
ion_flash_min_sort_next(
	ion_external_sort_cursor_t *cursor
);

#endif /* ION_FLASH_MIN_SORT_H_ */
