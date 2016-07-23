#include "flash_min_sort.h"

inline ion_err_t
ion_flash_min_sort_init(
	ion_external_sort_t				*es,
	FILE							*file,
	void							*buffer,
	ion_sort_comparator_context_t	context,
	ion_sort_comparator_t			compare_function,
	ion_key_size_t					key_size,
	ion_value_size_t				value_size,
	ion_page_size_t					page_size,
	ion_buffer_size_t				buffer_size
) {
#define ION_FILE_SORT_CEILING(numerator, denominator) (1 + (numerator - 1) / (denominator))

	uint32_t	num_pages_per_region;
	uint32_t	num_regions;
	long		next_index;

	if (0 != fseek(file, 0, SEEK_END)) {
		return err_file_bad_seek;
	}

	long file_size_in_bytes = ftell(file);

	if (-1 == file_size_in_bytes) {
		return err_file_bad_seek;
	}

	num_pages_per_region	= ION_FILE_SORT_CEILING(((uint32_t) file_size_in_bytes / page_size * key_size), (buffer_size - 2 * key_size - sizeof(next_index)));
	num_regions				= ION_FILE_SORT_CEILING(((uint32_t) file_size_in_bytes / page_size), (num_pages_per_region));

	rewind(file);

	return err_ok;
}

ion_err_t
ion_flash_min_sort_next(
	ion_external_sort_cursor_t *cursor
) {
	if (NULL == cursor->es->output_file) {}

	return err_ok;
}
