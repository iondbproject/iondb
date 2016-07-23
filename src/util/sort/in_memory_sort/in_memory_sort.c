#include "in_memory_sort.h"

void
ion_in_memory_swap(
	void				*data,
	ion_value_size_t	value_size,
	uint32_t			a,
	uint32_t			b
) {
	void	*a_offset	= (char *) data + value_size * a;
	void	*b_offset	= (char *) data + value_size * b;
	void	*tmp_buffer = alloca(value_size);

	memcpy(tmp_buffer, a_offset, value_size);
	memcpy(a_offset, b_offset, value_size);
	memcpy(b_offset, tmp_buffer, value_size);
}

int32_t
ion_in_memory_quick_sort_partition(
	void *data,
	ion_value_size_t value_size,
	int8_t (*compare_fcn)(void *data, ion_value_size_t value_size, uint32_t a, uint32_t b),
	int32_t low,
	int32_t high
) {
	int32_t pivot		= low;
	int32_t lower_bound = low - 1;
	int32_t upper_bound = high + 1;

	while (boolean_true) {
		do {
			upper_bound--;
		} while (compare_fcn(data, value_size, upper_bound, pivot) > 0);

		do {
			lower_bound++;
		} while (compare_fcn(data, value_size, lower_bound, pivot) < 0);

		if (lower_bound < upper_bound) {
			ion_in_memory_swap(data, value_size, lower_bound, upper_bound);
		}
		else {
			return upper_bound;
		}
	}
}

void
ion_in_memory_quick_sort_helper(
	void *data,
	uint32_t num_values,
	ion_value_size_t value_size,
	int8_t (*compare_fcn)(void *data, ion_value_size_t value_size, uint32_t a, uint32_t b),
	int32_t low,
	int32_t high
) {
	if (low < high) {
		int32_t pivot = ion_in_memory_quick_sort_partition(data, value_size, compare_fcn, low, high);

		ion_in_memory_quick_sort_helper(data, num_values, value_size, compare_fcn, low, pivot);
		ion_in_memory_quick_sort_helper(data, num_values, value_size, compare_fcn, pivot + 1, high);
	}
}

void
ion_in_memory_quick_sort(
	void *data,
	uint32_t num_values,
	ion_value_size_t value_size,
	int8_t (*compare_fcn)(void *data, ion_value_size_t value_size, uint32_t a, uint32_t b)
) {
	ion_in_memory_quick_sort_helper(data, num_values, value_size, compare_fcn, 0, num_values - 1);
}

ion_err_t
ion_in_memory_sort(
	void *data,
	uint32_t num_values,
	ion_value_size_t value_size,
	/* TODO: Put in proper comparator here */
	int8_t (*compare_fcn)(void *data, ion_value_size_t value_size, uint32_t a, uint32_t b),
	ion_in_memory_sort_algorithm_e sort_algorithm
) {
	switch (sort_algorithm) {
		case ION_IN_MEMORY_SORT_QUICK_SORT: {
			ion_in_memory_quick_sort(data, num_values, value_size, compare_fcn);
			break;
		}
	}

	return err_ok;
}
