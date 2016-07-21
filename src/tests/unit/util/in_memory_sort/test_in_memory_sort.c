/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Implementation of an in-place, recursive quicksort written by the author.
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

#include "test_in_memory_sort.h"
#include "../../../../key_value/kv_system.h"
#include "../../../../util/in_memory_sort/sort.h"

int8_t
quick_sort_int_comparator(
	void				*data,
	ion_value_size_t	value_size,
	uint32_t			a,
	uint32_t			b
) {
	UNUSED(value_size);

	/* Just assume are integers */
	int *data_as_int	= (int *) data;

	int result			= (data_as_int[a] - data_as_int[b]);

	if (result > 0) {
		return 1;
	}
	else if (result < 0) {
		return -1;
	}
	else {
		return 0;
	}
}

void
quick_sort_assert_sorted_order(
	planck_unit_test_t	*tc,
	void				*data,
	uint32_t			num_values,
	ion_value_size_t	value_size
) {
	uint32_t i;

	for (i = 0; i < num_values - 1; i++) {
		int8_t comp_result = quick_sort_int_comparator(data, value_size, i, i + 1);

		PLANCK_UNIT_ASSERT_TRUE(tc, comp_result <= 0);
	}
}

void
quick_sort_verify_result(
	planck_unit_test_t *tc,
	void *data,
	void *expected,
	uint32_t num_values,
	ion_value_size_t value_size,
	int8_t (*compare_fcn)(void *data, ion_value_size_t value_size, uint32_t a, uint32_t b)
) {
	ion_err_t err = ion_in_memory_sort(data, num_values, value_size, compare_fcn, ION_IN_MEMORY_SORT_QUICK_SORT);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, memcmp(data, expected, num_values * value_size));
	quick_sort_assert_sorted_order(tc, data, num_values, value_size);
}

void
test_quick_sort_single(
	planck_unit_test_t *tc
) {
	int data[]		= { 5 };
	int expected[]	= { 5 };

	quick_sort_verify_result(tc, data, expected, sizeof(data) / sizeof(int), sizeof(int), quick_sort_int_comparator);
}

void
test_quick_sort_sorted(
	planck_unit_test_t *tc
) {
	int data[]		= { 5, 6, 7, 8, 9 };
	int expected[]	= { 5, 6, 7, 8, 9 };

	quick_sort_verify_result(tc, data, expected, sizeof(data) / sizeof(int), sizeof(int), quick_sort_int_comparator);
}

void
test_quick_sort_sorted_reversed(
	planck_unit_test_t *tc
) {
	int data[]		= { 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20 };
	int expected[]	= { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };

	quick_sort_verify_result(tc, data, expected, sizeof(data) / sizeof(int), sizeof(int), quick_sort_int_comparator);
}

void
test_quick_sort_all_same(
	planck_unit_test_t *tc
) {
	int data[]		= { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };
	int expected[]	= { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };

	quick_sort_verify_result(tc, data, expected, sizeof(data) / sizeof(int), sizeof(int), quick_sort_int_comparator);
}

void
test_quick_sort_mix(
	planck_unit_test_t *tc
) {
	int data[]		= { 98, 7, -47, -58, 56, 18, 68, 36, 85, 34, 28, -8, 26, 51, -46, -73, -56, -40, -92, -73, -13, -28, -94, 36, -26, -27, -62, -50, -17, 22, 64, -5, -82, 70, 76, -54, 63, 53, 85, 27, 15, -5, 67, 66, 65, 52, -48, 27, -37, -83, 6, -93, 89, -35, 80, 15, -22, -60, 100, 94, -68, -81, 85, 12, 52, 52, -60, -16, 93, -48, 26, -53, 23, 82, 51, 95, -37, -47, -90, -26, 25, -79, 58, -19, -84, 7, 44, 35, 16, 8, 83, 43, -2, -14, 56, -75, 20, -74, -74, 65 };
	int expected[]	= { -94, -93, -92, -90, -84, -83, -82, -81, -79, -75, -74, -74, -73, -73, -68, -62, -60, -60, -58, -56, -54, -53, -50, -48, -48, -47, -47, -46, -40, -37, -37, -35, -28, -27, -26, -26, -22, -19, -17, -16, -14, -13, -8, -5, -5, -2, 6, 7, 7, 8, 12, 15, 15, 16, 18, 20, 22, 23, 25, 26, 26, 27, 27, 28, 34, 35, 36, 36, 43, 44, 51, 51, 52, 52, 52, 53, 56, 56, 58, 63, 64, 65, 65, 66, 67, 68, 70, 76, 80, 82, 83, 85, 85, 85, 89, 93, 94, 95, 98, 100 };

	quick_sort_verify_result(tc, data, expected, sizeof(data) / sizeof(int), sizeof(int), quick_sort_int_comparator);
}

planck_unit_suite_t *
in_memory_sort_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_quick_sort_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_quick_sort_sorted);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_quick_sort_sorted_reversed);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_quick_sort_all_same);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_quick_sort_mix);

	return suite;
}

void
runalltests_in_memory_sort(
) {
	planck_unit_suite_t *suite = in_memory_sort_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
