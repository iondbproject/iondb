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

#define ION_FMS_GET_FLAG(bit_vector_arr, flag_index) \
	((((uint8_t *)bit_vector_arr)[(flag_index) >> 3] >> ((flag_index) & 7)) & 1)

#define ION_FMS_SET_FLAG(bit_vector_arr, flag_index) \
	(((uint8_t *)bit_vector_arr)[(flag_index) >> 3] |= (1 << ((flag_index) & 7)))

#define ION_FMS_CLEAR_FLAG(bit_vector_arr, flag_index) \
	(((uint8_t *)bit_vector_arr)[(flag_index) >> 3] &= ~(1 << ((flag_index) & 7)))

#define ION_FMS_DIV_BY_POW_2_EXP(number, pow_2_exponent) \
	((number) >> (pow_2_exponent))

//#define ION_FLASH_MIN_SORT_GET_FLAG(byte, flag_index) \
//	((byte) >> ((flag_index) & 1))
//
//#define ION_FLASH_MIN_SORT_TOGGLE_FLAG(byte, flag_index) \
//	((byte) ^= (1 << (flag_index)))

//#define ION_FLASH_MIN_SORT_CUR_VALUE_FLAG_INDEX 0
//#define ION_FLASH_MIN_SORT_NEXT_VALUE_FLAG_INDEX 1

typedef struct {
	uint32_t	num_regions;
	uint32_t	num_pages_per_region;
	uint32_t	num_pages_last_region;
	uint32_t 	num_bytes_in_page;
	uint32_t	cur_region;
	uint32_t	cur_page;
	uint32_t	cur_page_in_region;
	uint16_t	cur_byte_in_page;
	uint32_t 	cur_byte_in_buffer;
	ion_external_sort_data_pointer_t		*cur_value;
	ion_external_sort_data_pointer_t		*temp_value;
	ion_external_sort_data_pointer_t		*min_index_bit_vector;
	ion_boolean_e	is_cur_null;
} ion_flash_min_sort_t;

/**
@brief		Initializes the minsort algorithm. This calculates the number of regions
			and size of the minimum index based on the number of values. It does a
			one pass scan through the file to initialize the minimum index.

@param[in]	es
 				An initialized external sort structure.
@param[in]	cursor
 				An initialized external sort cursor structure.
@return		An error defined in @ref ion_err_t.
*/
ion_err_t
ion_flash_min_sort_init(
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
ion_cursor_status_t
ion_flash_min_sort_next(
	ion_external_sort_cursor_t *cursor,
	void *output_value
);

#endif /* ION_FLASH_MIN_SORT_H_ */
