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
#include "external_sort_types.h"

ion_err_t
ion_flash_min_sort_init(
	ion_external_sort_t			*es,
	ion_external_sort_cursor_t	*cursor
) {
	ion_flash_min_sort_t *flash_min_sort_data = cursor->implementation_data;

	rewind(es->input_file);

	/* Calculate the number of regions and pages in each region. Note that the value instead of the key is stored in
	   the buckets of the minimal index to preserve generality. The size for the bit vectors to indicate uninitialized
	   value is also included in the calculation. */
	flash_min_sort_data->num_pages_per_region		= ION_EXTERNAL_SORT_CEILING(((uint32_t) es->num_pages * es->value_size), (cursor->buffer_size - 3 * es->value_size));

	flash_min_sort_data->num_regions				= ION_EXTERNAL_SORT_CEILING(((uint32_t) es->num_pages), (flash_min_sort_data->num_pages_per_region));

	flash_min_sort_data->last_page_in_last_region	= es->num_pages % flash_min_sort_data->num_pages_per_region;

	/* Do the initial pass. This requires scanning through the all of the values and finding the
	   minimum values in each region. */
	uint32_t	cur_region;
	uint32_t	cur_region_page;/* Current page in region */
	uint16_t	cur_page_byte;	/* Current byte in page */

	flash_min_sort_data->temp_value = (void *) (((uint8_t *) cursor->buffer) + flash_min_sort_data->num_regions * es->value_size);

	for (cur_region = 0; cur_region < flash_min_sort_data->num_regions; cur_region++) {
		for (cur_region_page = 0; cur_region_page < flash_min_sort_data->num_pages_per_region; cur_region_page++) {
			for (cur_page_byte = 0; cur_page_byte < es->page_size; cur_page_byte += es->value_size) {
				if ((cur_region_page == 0) && (cur_page_byte == 0)) {
					/* Insert the value into the index since it is the first value in the region. */
					memcpy((void *) (((uint8_t *) cursor->buffer) + cur_region * es->value_size), flash_min_sort_data->temp_value, es->value_size);
				}
				else if ((cur_region >= flash_min_sort_data->num_regions - 1) &&/* TODO: is it -1? */
						 (cur_region_page >= flash_min_sort_data->last_page_in_last_region) && (cur_page_byte >= es->num_records_last_page * es->value_size)) {
					/* This checks if the last record was reached. */
					break;
				}
				else {
					if (0 == fread(flash_min_sort_data->temp_value, es->value_size, 1, es->input_file)) {
						return err_file_read_error;
					}

					if (less_than == es->compare_function(es->context, flash_min_sort_data->temp_value, (void *) (((uint8_t *) cursor->buffer) + cur_region * es->value_size))) {
						memcpy((void *) (((uint8_t *) cursor->buffer) + cur_region * es->value_size), flash_min_sort_data->temp_value, es->value_size);
					}
				}
			}
		}
	}

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
