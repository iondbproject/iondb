#include "test_external_sort.h"

#define ION_EXTERNAL_SORT_SEED 0x8E01483F
#define ION_EXTERNAL_SORT_PAGE_SIZE 512

const char ion_external_sort_fname[] = "unsorted.dat";

typedef ion_err_t (*ion_external_sort_write_data_func_t)(FILE *unsorted_file, uint32_t num_values);

ion_comparison_e
ion_external_sort_uint16_comparator(
	ion_sort_comparator_context_t	*context,
	void							*value_a,
	void							*value_b
) {
	if (*((uint16_t *) value_a) > *((uint16_t *) value_b)) {
		return greater_than;
	}
	else if (*((uint16_t *) value_a) < *((uint16_t *) value_b)) {
		return less_than;
	}
	else {
		return equal;
	}
}

ion_err_t
ion_external_sort_write_uint16_data(
	FILE *unsorted_file,
	uint32_t num_values
) {
	srand(ION_EXTERNAL_SORT_SEED);

	uint32_t i;
	for (i = 0; i < num_values; i++) {
		uint16_t random = (uint16_t) rand();

		if (0 == fwrite(&random, sizeof(uint16_t), 1, unsorted_file)) {
			return err_file_read_error;
		}
	}

	return err_ok;
}

ion_comparison_e
ion_external_sort_3_char_str_comparator(
	ion_sort_comparator_context_t	*context,
	void							*value_a,
	void							*value_b
) {
	int comp_result = strncmp(value_a, value_b, 3);

	if (comp_result > 0) {
		return greater_than;
	}
	else if (comp_result < 0) {
		return less_than;
	}
	else {
		return equal;
	}
}

ion_err_t
ion_external_sort_write_3_char_str_data(
	FILE *unsorted_file,
	uint32_t num_values
) {
	srand(ION_EXTERNAL_SORT_SEED);
	char random_str[3];

	uint16_t values_per_page = 512 / 3;
	uint16_t cur_value_in_page = 0;

	uint32_t i;
	for (i = 0; i < num_values; i++) {
		random_str[0] = (char) rand();
		random_str[1] = (char) rand();
		random_str[2] = (char) rand();

		if (cur_value_in_page == values_per_page) {
			uint8_t num_padding_bytes = ION_EXTERNAL_SORT_PAGE_SIZE - values_per_page * 3;

			uint8_t j;
			for (j = 0; j < num_padding_bytes; j++) {
				if (0 == fwrite(&(uint8_t){0}, 1, 1, unsorted_file)) {
					return err_file_read_error;
				}
			}

			cur_value_in_page = 0;
		}

		if (0 == fwrite(random_str, 3, 1, unsorted_file)) {
			return err_file_read_error;
		}

		cur_value_in_page++;
	}

	return err_ok;
}

void
test_sort(
	planck_unit_test_t *tc,
	uint32_t num_values,
	uint32_t buffer_size,
	ion_value_size_t value_size,
	ion_boolean_e sorted_pages,
	ion_external_sort_algorithm_e sort_algorithm,
	ion_external_sort_write_data_func_t write_data_func,
	ion_sort_comparator_t comparator
) {
	FILE *unsorted_file = fopen(ion_external_sort_fname, "w+b");

	PLANCK_UNIT_ASSERT_TRUE(tc, unsorted_file != NULL);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, write_data_func(unsorted_file, num_values));

	ion_external_sort_t			es;
	ion_external_sort_cursor_t	cursor;
	uint8_t						buffer[buffer_size];

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, ion_external_sort_init(&es, unsorted_file, NULL, comparator, value_size, value_size, ION_EXTERNAL_SORT_PAGE_SIZE, sorted_pages, sort_algorithm));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, ion_external_sort_init_cursor(&es, &cursor, buffer, buffer_size));

	uint8_t	value_1[value_size];
	uint8_t	value_2[value_size];

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, cursor.next(&cursor, &value_1));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, cs_cursor_active, cursor.status);

	uint32_t i;

	for (i = 1; i < num_values; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, cursor.next(&cursor, &value_2));
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, cs_cursor_active, cursor.status);

		ion_comparison_e comparison_return_value = comparator(NULL, &value_1, &value_2);

		PLANCK_UNIT_ASSERT_TRUE(tc, less_than == comparison_return_value || equal == comparison_return_value);
		memcpy(value_1, value_2, value_size);
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, cursor.next(&cursor, &value_2));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, cs_end_of_results, cursor.status);

	ion_external_sort_destroy_cursor(&cursor);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, fclose(unsorted_file));
}

void
test_flash_min_sort_small_sort_record_at_a_time_with_page_aligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 1000;
	uint16_t buffer_size = 10;
	ion_value_size_t value_size = sizeof(uint16_t);
	ion_boolean_e sorted_pages = boolean_false;
	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_uint16_data;
	ion_sort_comparator_t comparator = ion_external_sort_uint16_comparator;

	test_sort(tc, num_records, buffer_size, value_size, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, comparator);
}

void
test_flash_min_sort_large_sort_record_at_a_time_with_page_aligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 10000;
	uint16_t buffer_size = 500;
	ion_value_size_t value_size = sizeof(uint16_t);
	ion_boolean_e sorted_pages = boolean_false;
	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_uint16_data;
	ion_sort_comparator_t comparator = ion_external_sort_uint16_comparator;

	test_sort(tc, num_records, buffer_size, value_size, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, comparator);
}

void
test_flash_min_sort_small_sort_record_at_a_time_with_page_unaligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 1000;
	uint16_t buffer_size = 10;
	ion_value_size_t value_size = 3;
	ion_boolean_e sorted_pages = boolean_false;
	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_3_char_str_data;
	ion_sort_comparator_t comparator = ion_external_sort_3_char_str_comparator;

	test_sort(tc, num_records, buffer_size, value_size, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, comparator);
}

void
test_flash_min_sort_large_sort_record_at_a_time_with_page_unaligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 10000;
	uint16_t buffer_size = 500;
	ion_value_size_t value_size = 3;
	ion_boolean_e sorted_pages = boolean_false;
	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_3_char_str_data;
	ion_sort_comparator_t comparator = ion_external_sort_3_char_str_comparator;

	test_sort(tc, num_records, buffer_size, value_size, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, comparator);
}

void
test_macro_get_flag(
	planck_unit_test_t *tc
) {
	uint8_t bit_vector[10];

	int i;
	for (i = 0; i < 10; i++) {
		bit_vector[i] = 0; // 0000 0000
	}

	bit_vector[1] = 0xFF; // 1111 1111
	bit_vector[9] = 0x23; // 0010 0011

	for (i = 0; i < 8 * 1; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, ION_FMS_GET_FLAG(bit_vector, i));
	}

	for (; i < 8 * 2; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, ION_FMS_GET_FLAG(bit_vector, i));
	}

	for (; i < 8 * 9; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, ION_FMS_GET_FLAG(bit_vector, i));
	}

	for (; i < 8 * 9 + 2; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, ION_FMS_GET_FLAG(bit_vector, i));
	}

	for (; i < 8 * 9 + 5; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, ION_FMS_GET_FLAG(bit_vector, i));
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, ION_FMS_GET_FLAG(bit_vector, i));
	i++;

	for (; i < 8 * 10; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, ION_FMS_GET_FLAG(bit_vector, i));
	}
}

void
test_macro_toggle_flag(
	planck_unit_test_t *tc
) {
	uint8_t bit_vector[10];

	int i;
	for (i = 0; i < 10; i++) {
		bit_vector[i] = 0; // 0000 0000
	}

	ION_FMS_SET_FLAG(bit_vector, 8 * 9); // bit_vector[9] = 0000 0001
	ION_FMS_SET_FLAG(bit_vector, 8 * 9 + 6); // bit_vector[9] = 0100 0001

	for (; i < 9; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, bit_vector[i]);
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0x41, bit_vector[9]);
}

planck_unit_suite_t *
file_sort_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_small_sort_record_at_a_time_with_page_aligned_data);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_large_sort_record_at_a_time_with_page_aligned_data);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_small_sort_record_at_a_time_with_page_unaligned_data);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_large_sort_record_at_a_time_with_page_unaligned_data);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_macro_get_flag);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_macro_toggle_flag);

	return suite;
}

void
runalltests_file_sort(
) {
	planck_unit_suite_t *suite = file_sort_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
