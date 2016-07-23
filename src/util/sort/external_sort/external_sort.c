/******************************************************************************/
/**
@file
@author		Wade Penson
@brief		External sorting algorithms for values stored in files.
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

#include "external_sort.h"
#include "../sort.h"

ion_err_t
ion_external_sort_init(
	ion_external_sort_t				*es,
	FILE							*input_file,
	void							*buffer,
	ion_sort_comparator_context_t	context,
	ion_sort_comparator_t			compare_function,
	ion_key_size_t					key_size,
	ion_value_size_t				value_size,
	ion_page_size_t					page_size,
	ion_buffer_size_t				buffer_size,
	ion_file_sort_algorithm_e		sort_algorithm
) {
	es->input_file			= input_file;
	es->buffer				= buffer;
	es->buffer_size			= buffer_size;
	es->context				= context;
	es->compare_function	= compare_function;
	es->key_size			= key_size;
	es->value_size			= value_size;
	es->page_size			= page_size;
	es->sort_algorithm		= sort_algorithm;

	es->output_file			= NULL;

	return err_ok;
}

ion_err_t
ion_external_sort_init_cursor(
	ion_external_sort_t			*es,
	ion_external_sort_cursor_t	*cursor
) {
	cursor->es = es;

	switch (es->sort_algorithm) {
		case ION_FILE_SORT_FLASH_MINSORT: {
			cursor->next = ion_flash_min_sort_next;
		}
	}

	return err_ok;
}

ion_err_t
ion_external_sort_dump_all(
	ion_external_sort_t *es,
	FILE				*output_file
) {
	es->output_file = output_file;

	ion_external_sort_cursor_t cursor;

	if (err_ok != ion_external_sort_init_cursor(es, &cursor)) {
		/* TODO: some error here */
	}

	return cursor.next(&cursor);
}
