#if !defined(ION_FLASH_MIN_SORT_H_)
#define ION_FLASH_MIN_SORT_H_

#include <stdio.h>
#include <stdint.h>
#include "external_sort.h"
#include "../sort.h"
#include "../../../key_value/kv_system.h"

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
);

ion_err_t
ion_flash_min_sort_next(
	ion_external_sort_cursor_t *cursor
);

#endif /* ION_FLASH_MIN_SORT_H_ */
