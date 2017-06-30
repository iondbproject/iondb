/******************************************************************************/
/**
@file
@author		Wade Penson
@brief		Structures and other types used by the external sorting algorithms.
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

#if !defined(ION_EXTERNAL_SORT_TYPES_H_)
#define ION_EXTERNAL_SORT_TYPES_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include "../sort.h"
#include "../../../key_value/kv_system.h"

#if defined(ARDUINO)
#include "../../../serial/printf_redirect.h"
#include "../../../serial/serial_c_iface.h"
#include "../../../file/kv_stdio_intercept.h"
#include "../../../file/SD_stdio_c_iface.h"
#endif

#define ION_EXTERNAL_SORT_CEILING(numerator, denominator) (1 + (numerator - 1) / (denominator))

typedef uint8_t ion_external_sort_data_pointer_t;

typedef enum {
	ION_FILE_SORT_FLASH_MINSORT
} ion_external_sort_algorithm_e;

typedef struct ion_external_sort_cursor_s ion_external_sort_cursor_t;

typedef struct {
	FILE							*input_file;
	ion_sort_comparator_context_t	context;
	ion_sort_comparator_t			compare_function;
	ion_key_size_t					key_size;
	ion_value_size_t				value_size;
	ion_page_size_t					page_size;
	uint32_t						num_pages;
	uint16_t						num_values_last_page;
	ion_boolean_e					sorted_pages;
	ion_external_sort_algorithm_e	sort_algorithm;
} ion_external_sort_t;

struct ion_external_sort_cursor_s {
	ion_external_sort_t					*es;
	void								*implementation_data;
	FILE								*output_file;
	ion_external_sort_data_pointer_t	*buffer;
	ion_buffer_size_t					buffer_size;
	ion_cursor_status_t					status;

	ion_err_t							(*next)(
		ion_external_sort_cursor_t	*cursor,
		void						*output_value
	);
};

#if defined(__cplusplus)
}
#endif

#endif /* ION_EXTERNAL_SORT_TYPES_H_ */
