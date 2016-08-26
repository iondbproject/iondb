#include "in_memory_sort.h"

void
ion_in_memory_swap(
	ion_in_memory_sort_t *ims,
	uint32_t			a,
	uint32_t			b
) {
	void	*a_offset	= ((char *) ims->data) + ims->value_size * a;
	void	*b_offset	= ((char *) ims->data) + ims->value_size * b;
	void	*tmp_buffer = alloca(ims->value_size);

	memcpy(tmp_buffer, a_offset, ims->value_size);
	memcpy(a_offset, b_offset, ims->value_size);
	memcpy(b_offset, tmp_buffer, ims->value_size);
}

uint32_t
ion_in_memory_quick_sort_partition(
	ion_in_memory_sort_t *ims,
	uint32_t low,
	uint32_t high
) {
	uint32_t pivot		= low;
	uint32_t lower_bound = low - 1;
	uint32_t upper_bound = high + 1;

	while (boolean_true) {
		do {
			upper_bound--;
		} while (greater_than == ims->compare_function(ims, ((char *) ims->data) + ims->value_size * upper_bound, ((char *) ims->data) + ims->value_size * pivot));

		do {
			lower_bound++;
		} while (less_than == ims->compare_function(ims, ((char *) ims->data) + ims->value_size * lower_bound, ((char *) ims->data) + ims->value_size * pivot));

		if (lower_bound < upper_bound) {
			ion_in_memory_swap(ims, lower_bound, upper_bound);
		}
		else {
			return upper_bound;
		}
	}
}

void
ion_in_memory_quick_sort_helper(
	ion_in_memory_sort_t *ims,
	uint32_t low,
	uint32_t high
) {
	if (low < high) {
		uint32_t pivot = ion_in_memory_quick_sort_partition(ims, low, high);

		ion_in_memory_quick_sort_helper(ims, low, pivot);
		ion_in_memory_quick_sort_helper(ims, pivot + 1, high);
	}
}

void
ion_in_memory_quick_sort(
	ion_in_memory_sort_t *ims
) {
	ion_in_memory_quick_sort_helper(ims, 0, ims->num_values - 1);
}

void
ion_in_memory_sort_init(
	ion_in_memory_sort_t *ims,
	void *data,
	uint32_t num_values,
	ion_value_size_t value_size,
	ion_sort_comparator_context_t	context,
	ion_sort_comparator_t			compare_function,
	ion_in_memory_sort_algorithm_e sort_algorithm
) {
	ims->data = data;
	ims->num_values = num_values;
	ims->value_size = value_size;
	ims->context = context;
	ims->compare_function = compare_function;
	ims->sort_algorithm = sort_algorithm;
}

ion_err_t
ion_in_memory_sort_run(
	ion_in_memory_sort_t *ims
) {
	switch (ims->sort_algorithm) {
		case ION_IN_MEMORY_SORT_QUICK_SORT: {
			ion_in_memory_quick_sort(ims);
			break;
		}
	}

	return err_ok;
}
