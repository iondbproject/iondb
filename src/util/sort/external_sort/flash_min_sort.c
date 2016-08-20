/******************************************************************************/
/**
@file
@author		Wade Penson
@brief		Implementation of the flash minsort algorithm.
@details	It does not use dynamic sizing of the minimum index since the
 			number of values is already known.
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

@todo		Caching pages
@todo		Code for sorted regions
*/
/******************************************************************************/

#include "flash_min_sort.h"
#include "external_sort_types.h"

#if defined(DEGUB)
uint32_t page_reads;
#endif

ion_err_t
ion_flash_min_sort_init(
	ion_external_sort_t			*es,
	ion_external_sort_cursor_t	*cursor
) {
#if defined(DEGUB)
	page_reads = 0;
#endif

	ion_flash_min_sort_t *fms = cursor->implementation_data;

	/* Calculate the number of regions and pages in each region. Note that the value instead of the key is stored in
	   the buckets of the minimal index to preserve generality. The size for the bit vectors to indicate uninitialized
	   value is also included in the calculation. */
	fms->num_regions = cursor->buffer_size - (es->page_size + 4) - 2 * es->value_size;

	if (NULL != cursor->output_file) {
		fms->num_regions -= (es->page_size + 4);
	}

	fms->num_regions = (fms->num_regions * 8) / (es->value_size * 8 + 1);
	fms->num_regions = (fms->num_regions > es->num_pages) ? es->num_pages : fms->num_regions;

	fms->num_pages_per_region	= ION_EXTERNAL_SORT_CEILING(((uint32_t) es->num_pages), (fms->num_regions));

	fms->num_cache_pages = ((int32_t) cursor->buffer_size - ((int32_t) fms->num_regions * es->value_size + 2 * es->value_size + ION_EXTERNAL_SORT_CEILING(fms->num_regions, 8))) / (es->page_size + 4);

	/* Initialize pointers to the corresponding locations in the buffer. */
	fms->cur_value				= cursor->buffer + fms->num_regions * es->value_size;
	fms->temp_value				= fms->cur_value + es->value_size; // TODO: Does not need to be allocated
	fms->min_index_bit_vector	= fms->temp_value + es->value_size;
	fms->page_statuses			= (uint32_t *) fms->min_index_bit_vector + ION_EXTERNAL_SORT_CEILING(fms->num_regions, 8);
	fms->cache_pages			= (ion_external_sort_data_pointer_t *) fms->page_statuses + fms->num_cache_pages * 4;

	/* Set the bits to 0 in the minimum index bit vector. */
	uint32_t i;

	for (i = 0; i < ION_EXTERNAL_SORT_CEILING(fms->num_regions, 8); i++) {
		(fms->min_index_bit_vector)[i] = 0xFF;
	}

	/* Do the initial pass. This requires scanning through the all of the values and finding the
	   minimum values in each region. */
	fms->cur_page			= 0;
	fms->cur_byte_in_page	= 0;
	fms->cur_region			= 0;
	fms->cur_page_in_region = 0;
	fms->cur_byte_in_buffer = 0;

	fms->num_bytes_in_page	= (es->page_size / (uint32_t) es->value_size) * (uint32_t) es->value_size;

	if (0 != fseek(es->input_file, 0, SEEK_SET)) {
		return err_file_bad_seek;
	}

	for (fms->cur_page = 0; fms->cur_page < es->num_pages; fms->cur_page++) {
		/* If it is the last page, change the number of value to loop through */
		if (fms->cur_page == es->num_pages - 1) {
			fms->num_bytes_in_page = (uint32_t) es->num_values_last_page * es->value_size;
		}

		for (fms->cur_byte_in_page = 0; fms->cur_byte_in_page < fms->num_bytes_in_page; fms->cur_byte_in_page += es->value_size) {
			if (0 == fread(fms->temp_value, es->value_size, 1, es->input_file)) {
				return err_file_read_error;
			}

			if (((0 == fms->cur_page_in_region) && (0 == fms->cur_byte_in_page)) || (less_than == es->compare_function(es->context, fms->temp_value, cursor->buffer + fms->cur_byte_in_buffer))) {
				memcpy(cursor->buffer + fms->cur_byte_in_buffer, fms->temp_value, es->value_size);
			}
		}

		/* If it is not the last page, progress page/region and seek to next page boundary. */
		if (fms->cur_page < es->num_pages - 1) {
			if (fms->cur_page_in_region == fms->num_pages_per_region - 1) {
				fms->cur_region++;
				fms->cur_page_in_region = 0;
				fms->cur_byte_in_buffer += es->value_size;
			}
			else {
				fms->cur_page_in_region++;
			}

			/* Seek to beginning of the next page. */
			if (0 != fseek(es->input_file, es->page_size - (fms->cur_byte_in_page), SEEK_CUR)) {
				return err_file_bad_seek;
			}
		}
	}

	fms->num_bytes_in_page	= (es->page_size / (uint32_t) es->value_size) * (uint32_t) es->value_size;

	fms->cur_byte_in_page	= 0;
	fms->cur_page_in_region = 0;
	fms->is_cur_null = boolean_false;
	fms->page_statuses[0] = 0xFFFFFFFF;
	fms->page_statuses[1] = 0;

	cursor->status = cs_cursor_initialized;
	return err_ok;
}

ion_cursor_status_t
ion_flash_min_sort_next(
	ion_external_sort_cursor_t	*cursor,
	void						*output_value
) {
	ion_flash_min_sort_t	*fms	= cursor->implementation_data;
	ion_external_sort_t		*es		= cursor->es;

	while (boolean_false == fms->is_cur_null) {
		if (0 == fms->cur_page_in_region && 0 == fms->cur_byte_in_page) {
			fms->is_cur_null = boolean_true;

			uint32_t temp_page = 0;
			uint32_t temp_region = 0;
			uint16_t temp_byte_in_buffer = 0;

			while (temp_region < fms->num_regions) {
				if (0 != ION_FMS_GET_FLAG(fms->min_index_bit_vector, temp_region) && (boolean_true == fms->is_cur_null || greater_than == es->compare_function(es->context, fms->cur_value, cursor->buffer + temp_byte_in_buffer))) {
					fms->is_cur_null = boolean_false;
					memcpy(fms->cur_value, cursor->buffer + temp_byte_in_buffer, es->value_size);
					fms->cur_region = temp_region;
					fms->cur_page = temp_page;
					fms->cur_byte_in_buffer = temp_byte_in_buffer;
				}

				temp_page += fms->num_pages_per_region;
				temp_region++;
				temp_byte_in_buffer += es->value_size;
			}

			if (boolean_true == fms->is_cur_null) {
				break;
			}

			/* Seek to the beginning of the region. */
			if (0 != fseek(es->input_file, fms->cur_page * es->page_size, SEEK_SET)) {
				return err_file_bad_seek;
			}

			/* fms->min_index_bit_vector[current_region] = NULL */
			ION_FMS_CLEAR_FLAG(fms->min_index_bit_vector, fms->cur_region);
		}

		while (fms->cur_page_in_region < fms->num_pages_per_region) {
			if (fms->cur_page >= es->num_pages - 1) {
				if (fms->cur_page > es->num_pages - 1) {
					fms->cur_page_in_region = 0;
					break;
				}

				fms->num_bytes_in_page = es->num_values_last_page * (uint32_t) es->value_size;
			}

			if (fms->cur_page != fms->page_statuses[0]) {
				if (0 == fread(fms->cache_pages, fms->num_bytes_in_page, 1, es->input_file)) {
					return err_file_read_error;
				}

#if defined(DEGUB)
				page_reads++;
#endif

				fms->page_statuses[0] = fms->cur_page;
			}

			if (boolean_true == es->sorted_pages && 0 == fms->cur_byte_in_page) {
				/* Binary search */
				uint16_t lower_bound = 0;
				uint16_t upper_bound = fms->num_bytes_in_page / es->value_size;

				while (1) {
					if (upper_bound < lower_bound) {
						return err_item_not_found;
					}

					uint16_t mid_point = lower_bound + (upper_bound - lower_bound) / 2;

					if (less_than == es->compare_function(es->context, fms->cache_pages + mid_point * es->value_size, fms->cur_value)) {
						lower_bound = mid_point + 1;
					}

					if (greater_than == es->compare_function(es->context, fms->cache_pages + mid_point * es->value_size, fms->cur_value)) {
						upper_bound = mid_point - 1;
					}

					if (equal == es->compare_function(es->context, fms->cache_pages + mid_point * es->value_size, fms->cur_value)) {
						fms->cur_byte_in_page = mid_point * es->value_size;
						break;
					}
				}
			}

			while (fms->cur_byte_in_page < fms->num_bytes_in_page) {
				fms->temp_value = fms->cache_pages + fms->cur_byte_in_page;

				if (equal == es->compare_function(es->context, fms->temp_value, fms->cur_value)) {
					if (NULL != cursor->output_file) {
						/* Dump values to file */
						fms->page_statuses[1] += es->value_size;

						if (fms->page_statuses[1] > es->page_size) {
							memset(fms->cache_pages + es->page_size + fms->page_statuses[1] - es->value_size, 0, es->page_size - (fms->page_statuses[1] - es->value_size));

							if (0 == fwrite(fms->cache_pages + es->page_size, es->page_size, 1, cursor->output_file)) {
								return err_file_write_error;
							}

							fms->page_statuses[1] = es->value_size;
						}

						memcpy(fms->cache_pages + es->page_size + fms->page_statuses[1] - es->value_size, fms->temp_value, es->value_size);
					}
					else {
						fms->cur_byte_in_page += es->value_size;
						memcpy(output_value, fms->temp_value, es->value_size);
						cursor->status = cs_cursor_active;
						return err_ok;
					}
				}
				else if (greater_than == es->compare_function(es->context, fms->temp_value, fms->cur_value) && (0 == ION_FMS_GET_FLAG(fms->min_index_bit_vector, fms->cur_region) || less_than == es->compare_function(es->context, fms->temp_value, cursor->buffer + fms->cur_byte_in_buffer))) {
					memcpy(cursor->buffer + fms->cur_byte_in_buffer, fms->temp_value, es->value_size);
					ION_FMS_SET_FLAG(fms->min_index_bit_vector, fms->cur_region);
				}

				if (boolean_true == es->sorted_pages) {
					break;
				}

				fms->cur_byte_in_page += es->value_size;
			}

			if (fms->cur_page != es->num_pages) {
				if (0 != fseek(es->input_file, es->page_size - fms->cur_byte_in_page, SEEK_CUR)) {
					return err_file_bad_seek;
				}
			}

			fms->cur_byte_in_page = 0;
			fms->cur_page++;
			fms->cur_page_in_region++;
		}

		if (fms->cur_page == es->num_pages) {
			fms->num_bytes_in_page = (es->page_size / (uint32_t) es->value_size) * (uint32_t) es->value_size;
		}

		fms->cur_page = 0;
		fms->cur_page_in_region = 0;
		fms->cur_byte_in_page = 0;
	}

#if defined(DEGUB)
	printf("%d\n", page_reads);
#endif

	if (NULL != cursor->output_file) {
		if (0 == fwrite(fms->cache_pages + es->page_size, es->num_values_last_page * es->value_size, 1, cursor->output_file)) {
			return err_file_write_error;
		}

		if (0 != fseek(cursor->output_file, 0, SEEK_SET)) {
			return err_file_bad_seek;
		}
	}

	cursor->status = cs_end_of_results;
	return err_ok;
}
