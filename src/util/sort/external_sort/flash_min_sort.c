/******************************************************************************/
/**
@file
@author		Wade Penson
@brief		Implementation of the flash minsort algorithm that does not use
			dynamic sizing of the minimum index since the number of values
			is already known.
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

#include "flash_min_sort.h"

ion_err_t
ion_flash_min_sort_init(
	ion_external_sort_t			*es,
	ion_external_sort_cursor_t	*cursor
) {
#define ION_FILE_SORT_CEILING(numerator, denominator) (1 + (numerator - 1) / (denominator))

	ion_flash_min_sort_t *flash_min_sort_data = cursor->implementation_data;

	if (0 != fseek(es->input_file, 0, SEEK_END)) {
		return err_file_bad_seek;
	}

	long file_size_in_bytes = ftell(es->input_file);

	if (-1 == file_size_in_bytes) {
		return err_file_bad_seek;
	}

	flash_min_sort_data->num_pages_per_region	= ION_FILE_SORT_CEILING(((uint32_t) file_size_in_bytes / es->page_size * es->key_size), (es->buffer_size));
	flash_min_sort_data->num_regions			= ION_FILE_SORT_CEILING(((uint32_t) file_size_in_bytes / es->page_size), (flash_min_sort_data->num_pages_per_region));

	rewind(es->input_file);

	return err_ok;
}

ion_err_t
ion_flash_min_sort_next(
	ion_external_sort_cursor_t *cursor
) {
	ion_flash_min_sort_t *flash_min_sort_data = cursor->implementation_data;

	if (NULL == cursor->output_file) {}

	return err_ok;
}
