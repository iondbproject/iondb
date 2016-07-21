#include "file_sort.h"

ion_err_t
ion_file_sort(
	FILE						*file,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_page_size_t				page_size,
	void						*buffer,
	ion_buffer_size_t			buffer_size,
#if defined(ION_TABLE)

#else

#endif
	ion_file_sort_algorithm_e	sort_algorithm	/* TODO: Support for merge sort */
) {
#define ION_FILE_SORT_CEILING(numerator, denominator) (1 + (numerator - 1) / (denominator))

	uint32_t	num_pages_per_region;
	uint32_t	num_regions;
	long		next_index;

	if (0 != fseek(file, 0L, SEEK_END)) {
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
