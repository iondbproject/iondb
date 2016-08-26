#if !defined(ION_IN_MEMORY_SORT_H_)
#define ION_IN_MEMORY_SORT_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <alloca.h>
#include "../sort.h"
#include "../../../key_value/kv_system.h"

typedef enum {
	ION_IN_MEMORY_SORT_QUICK_SORT
} ion_in_memory_sort_algorithm_e;

typedef struct {
	void *data;
	uint32_t num_values;
	ion_value_size_t value_size;
	ion_sort_comparator_context_t	context;
	ion_sort_comparator_t			compare_function;
	ion_in_memory_sort_algorithm_e sort_algorithm;
} ion_in_memory_sort_t;

void
ion_in_memory_sort_init(
	ion_in_memory_sort_t *ims,
	void *data,
	uint32_t num_values,
	ion_value_size_t value_size,
	ion_sort_comparator_context_t	context,
	ion_sort_comparator_t			compare_function,
	ion_in_memory_sort_algorithm_e sort_algorithm
);

ion_err_t
ion_in_memory_sort_run(
	ion_in_memory_sort_t *ims
);

#if defined(__cplusplus)
}
#endif

#endif /* ION_IN_MEMORY_SORT_H_ */
