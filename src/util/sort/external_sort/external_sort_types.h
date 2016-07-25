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

typedef enum {
	ION_FILE_SORT_FLASH_MINSORT
} ion_file_sort_algorithm_e;

typedef struct ion_external_sort_cursor_s ion_external_sort_cursor_t;

typedef struct {
	FILE							*input_file;
	void							*buffer;
	ion_buffer_size_t				buffer_size;
	ion_sort_comparator_context_t	context;
	ion_sort_comparator_t			compare_function;
	ion_key_size_t					key_size;
	ion_value_size_t				value_size;
	ion_page_size_t					page_size;
	ion_file_sort_algorithm_e		sort_algorithm;
} ion_external_sort_t;

struct ion_external_sort_cursor_s {
	ion_external_sort_t *es;
	FILE				*output_file;
	void				*implementation_data;

	ion_err_t			(*next)(
		ion_external_sort_cursor_t *es
	);
};

#if defined(__cplusplus)
}
#endif

#endif /* ION_EXTERNAL_SORT_TYPES_H_ */
