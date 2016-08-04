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
#include "external_sort_types.h"

ion_err_t
ion_external_sort_init(
	ion_external_sort_t				*es,
	FILE							*input_file,
	ion_sort_comparator_context_t	context,
	ion_sort_comparator_t			compare_function,
	ion_key_size_t					key_size,
	ion_value_size_t				value_size,
	ion_page_size_t					page_size,
	ion_boolean_e					sorted_pages,
	ion_external_sort_algorithm_e		sort_algorithm
) {
	es->input_file			= input_file;
	es->context				= context;
	es->compare_function	= compare_function;
	es->key_size			= key_size;
	es->value_size			= value_size;
	es->page_size			= page_size;
	es->sorted_pages		= sorted_pages;
	es->sort_algorithm		= sort_algorithm;

	if (0 != fseek(es->input_file, 0, SEEK_END)) {
		return err_file_bad_seek;
	}

	long file_size_in_bytes = ftell(es->input_file);

	if (-1 == file_size_in_bytes) {
		return err_file_bad_seek;
	}

	es->num_values_last_page = (uint16_t) ((file_size_in_bytes % es->page_size) / es->value_size);
	if (0 == es->num_values_last_page) {
		es->num_values_last_page = es->page_size / (uint16_t) es->value_size; // TODO
	}

	es->num_pages = ION_EXTERNAL_SORT_CEILING((uint32_t) file_size_in_bytes, es->page_size);

	return err_ok;
}

ion_err_t
ion_external_sort_init_cursor(
	ion_external_sort_t			*es,
	ion_external_sort_cursor_t	*cursor,
	void						*buffer,
	ion_buffer_size_t			buffer_size
) {
	cursor->es			= es;
	cursor->output_file = NULL;
	cursor->buffer		= buffer;
	cursor->buffer_size = buffer_size;

	switch (es->sort_algorithm) {
		case ION_FILE_SORT_FLASH_MINSORT: {
			cursor->implementation_data = malloc(sizeof(ion_flash_min_sort_t));

			if (NULL == cursor->implementation_data) {
				return err_out_of_memory;
			}

			cursor->next = ion_flash_min_sort_next;
			return ion_flash_min_sort_init(es, cursor);
		}

		default: {
			return err_ok;
		}
	}
}

void
ion_external_sort_destroy_cursor(
	ion_external_sort_cursor_t *cursor
) {
	free(cursor->implementation_data);
}

ion_err_t
ion_external_sort_dump_all(
	ion_external_sort_t *es,
	FILE				*output_file,
	void				*buffer,
	ion_buffer_size_t	buffer_size
) {
	ion_external_sort_cursor_t cursor;

	cursor.es			= es;
	cursor.output_file	= output_file;
	cursor.buffer		= buffer;
	cursor.buffer_size	= buffer_size;

	void *value = malloc(es->value_size);

	if (NULL == value) {
		return err_out_of_memory;
	}

	ion_err_t error = err_ok;

	switch (es->sort_algorithm) {
		case ION_FILE_SORT_FLASH_MINSORT: {
			cursor.next = ion_flash_min_sort_next;

			if (err_ok != (error = ion_flash_min_sort_init(es, &cursor))) {
				break;
			}

			ion_flash_min_sort_t flash_min_sort_data;
			cursor.implementation_data = &flash_min_sort_data;

			error = cursor.next(&cursor, value);
			break;
		}
		default: {
			break;
		}
	}

	free(value);
	return error;
}
