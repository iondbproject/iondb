#if !defined(ION_IN_MEMORY_SORT_H_)
#define ION_IN_MEMORY_SORT_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <alloca.h>
#include "../../../key_value/kv_system.h"

typedef enum {
	ION_IN_MEMORY_SORT_QUICK_SORT
} ion_in_memory_sort_algorithm_e;

ion_err_t
ion_in_memory_sort(
	void *data,
	uint32_t num_values,
	ion_value_size_t value_size,
	/* TODO: Put in proper comparator here */
	int8_t (*compare_fcn)(void *data, ion_value_size_t value_size, uint32_t a, uint32_t b),
	ion_in_memory_sort_algorithm_e sort_algorithm
);

#if defined(__cplusplus)
}
#endif

#endif /* ION_IN_MEMORY_SORT_H_ */
