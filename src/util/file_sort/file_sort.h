#if !defined(ION_FILE_SORT_H_)
#define ION_FILE_SORT_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include "../../key_value/kv_system.h"

typedef enum {
	ION_FILE_SORT_FLASH_MINSORT
} ion_file_sort_algorithm_e;

ion_err_t ion_file_sort(FILE * file, ion_key_size_t key_size, ion_value_size_t value_size, ion_page_size_t page_size, void *buffer, ion_buffer_size_t buffer_size,
#if defined(ION_TABLE)

#else

#endif
						ion_file_sort_algorithm_e sort_algorithm);

#if defined(__cplusplus)
}
#endif

#endif /* ION_FILE_SORT_H_ */
