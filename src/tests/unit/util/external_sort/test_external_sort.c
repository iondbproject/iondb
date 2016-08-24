#include "test_external_sort.h"
#include "../../../../util/sort/external_sort/external_sort_types.h"

#define ION_EXTERNAL_SORT_SEED 0x8E01483F
#define ION_EXTERNAL_SORT_PAGE_SIZE 512

const char ion_external_sort_unsorted_fname[] = "unsorted.dat";
const char ion_external_sort_sorted_fname[] = "sorted.dat";

typedef ion_err_t (*ion_external_sort_write_data_func_t)(FILE *unsorted_file, uint32_t num_values);
typedef void (*ion_external_sort_sorted_value_func_t)(uint32_t index, void *value);

ion_comparison_e
ion_external_sort_uint16_comparator(
	ion_sort_comparator_context_t	context,
	void							*value_a,
	void							*value_b
) {
	UNUSED(context);

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

void
ion_external_sort_expected_uint16_sequential_data(
	uint32_t index,
	void *value
) {
	memcpy(value, &(uint16_t){(uint16_t) index}, sizeof(uint16_t));
	return;
}

ion_err_t
ion_external_sort_write_uint16_random_data(
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

ion_err_t
ion_external_sort_write_uint16_sequential_data(
	FILE *unsorted_file,
	uint32_t num_values
) {
	int32_t i;
	for (i = num_values - 1; i >= 0; i--) {
		if (0 == fwrite(&i, sizeof(uint16_t), 1, unsorted_file)) {
			return err_file_read_error;
		}
	}

	return err_ok;
}

ion_comparison_e
ion_external_sort_5_char_str_comparator(
	ion_sort_comparator_context_t	context,
	void							*value_a,
	void							*value_b
) {
	UNUSED(context);

	int comp_result = strncmp(value_a, value_b, 5);

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
ion_external_sort_write_5_char_str_random_data(
	FILE *unsorted_file,
	uint32_t num_values
) {
	srand(ION_EXTERNAL_SORT_SEED);
	char random_str[5];

	uint16_t values_per_page = 512 / 5;
	uint16_t cur_value_in_page = 0;

	uint32_t i;

	for (i = 0; i < num_values; i++) {
		uint8_t j;
		for (j = 0; j < 5; j++) {
			random_str[j] = (char) rand();
		}

		if (cur_value_in_page == values_per_page) {
			uint8_t num_padding_bytes = ION_EXTERNAL_SORT_PAGE_SIZE - values_per_page * 5;

			for (j = 0; j < num_padding_bytes; j++) {
				if (0 == fwrite(&(uint8_t){0}, 1, 1, unsorted_file)) {
					return err_file_read_error;
				}
			}

			cur_value_in_page = 0;
		}

		if (0 == fwrite(random_str, 5, 1, unsorted_file)) {
			return err_file_read_error;
		}

		cur_value_in_page++;
	}

	return err_ok;
}

void
ion_external_sort_expected_5_char_str_sequential_data(
	uint32_t index,
	void *value
) {
	char temp_str[6];
	sprintf(temp_str, "%05d", index);
	memcpy(value, temp_str, 5);
}

ion_err_t
ion_external_sort_write_5_char_str_sequential_data(
	FILE *unsorted_file,
	uint32_t num_values,
	ion_boolean_e ascending
) {
	srand(ION_EXTERNAL_SORT_SEED);
	char temp_str[6];

	uint16_t values_per_page = 512 / 5;
	uint16_t cur_value_in_page = 0;

	uint32_t i;

	for (i = 0; i < num_values; i++) {
		if (boolean_true == ascending) {
			sprintf(temp_str, "%05d", i);
		}
		else {
			sprintf(temp_str, "%05d", num_values - i - 1);
		}

		if (cur_value_in_page == values_per_page) {
			uint8_t num_padding_bytes = ION_EXTERNAL_SORT_PAGE_SIZE - values_per_page * 5;

			uint8_t j;
			for (j = 0; j < num_padding_bytes; j++) {
				if (0 == fwrite(&(uint8_t){0}, 1, 1, unsorted_file)) {
					return err_file_read_error;
				}
			}

			cur_value_in_page = 0;
		}

		if (0 == fwrite(temp_str, 5, 1, unsorted_file)) {
			return err_file_read_error;
		}

		cur_value_in_page++;
	}

	return err_ok;
}

ion_err_t
ion_external_sort_write_5_char_str_sequential_data_ascending(
	FILE *unsorted_file,
	uint32_t num_values
) {
	return ion_external_sort_write_5_char_str_sequential_data(unsorted_file, num_values, boolean_true);
}

ion_err_t
ion_external_sort_write_5_char_str_sequential_data_descending(
	FILE *unsorted_file,
	uint32_t num_values
) {
	return ion_external_sort_write_5_char_str_sequential_data(unsorted_file, num_values, boolean_false);
}

void
test_sort(
	planck_unit_test_t *tc,
	uint32_t num_values,
	uint32_t max_buffer_size,
	ion_value_size_t value_size,
	ion_boolean_e dump_all,
	ion_boolean_e sorted_pages,
	ion_external_sort_algorithm_e sort_algorithm,
	ion_external_sort_write_data_func_t write_data_func,
	ion_external_sort_sorted_value_func_t sorted_value_func,
	ion_sort_comparator_t comparator
) {
	FILE *unsorted_file = fopen(ion_external_sort_unsorted_fname, "w+b");
	PLANCK_UNIT_ASSERT_TRUE(tc, unsorted_file != NULL);

	FILE *sorted_file = NULL;

	if (boolean_true == dump_all) {
		sorted_file = fopen(ion_external_sort_sorted_fname, "w+b");
		PLANCK_UNIT_ASSERT_TRUE(tc, sorted_file != NULL);
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, write_data_func(unsorted_file, num_values));

	ion_external_sort_t			es;
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, ion_external_sort_init(&es, unsorted_file, sorted_file, comparator, value_size, value_size, ION_EXTERNAL_SORT_PAGE_SIZE, sorted_pages, sort_algorithm));

	uint32_t buffer_size = ion_external_sort_bytes_of_memory_required(&es, max_buffer_size, dump_all);

	ion_external_sort_cursor_t	cursor;
	uint8_t						buffer[buffer_size];

	uint8_t	value_1[value_size];
	uint8_t	value_2[value_size];

	if (boolean_true == dump_all) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, ion_external_sort_dump_all(&es, sorted_file, buffer, buffer_size));
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, fread(value_1, value_size, 1, sorted_file));
	}
	else {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, ion_external_sort_init_cursor(&es, &cursor, buffer, buffer_size));
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, cursor.next(&cursor, value_1));
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, cs_cursor_active, cursor.status);
	}

	if (NULL != sorted_value_func) {
		sorted_value_func(0, value_2);
		ion_comparison_e comparison_return_value = comparator(NULL, value_1, value_2);
		PLANCK_UNIT_ASSERT_TRUE(tc, less_than == comparison_return_value || equal == comparison_return_value);
	}

	uint32_t i;

	uint32_t cur_value_in_page = 1;
	uint32_t values_per_page = es.page_size / value_size;


	for (i = 1; i < num_values; i++) {
		if (boolean_true == dump_all) {
			if (cur_value_in_page == values_per_page) {
				PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, fseek(sorted_file, es.page_size - values_per_page * value_size, SEEK_CUR));
				cur_value_in_page = 1;
			}
			else {
				cur_value_in_page++;
			}

			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, fread(value_2, value_size, 1, sorted_file));
		}
		else {
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, cursor.next(&cursor, value_2));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, cs_cursor_active, cursor.status);
		}

		if (NULL == sorted_value_func) {
			ion_comparison_e comparison_return_value = comparator(NULL, value_1, value_2);
			PLANCK_UNIT_ASSERT_TRUE(tc, less_than == comparison_return_value || equal == comparison_return_value);
			memcpy(value_1, value_2, value_size);
		}
		else {
			sorted_value_func(i, value_1);
			ion_comparison_e comparison_return_value = comparator(NULL, value_1, value_2);
			PLANCK_UNIT_ASSERT_TRUE(tc, equal == comparison_return_value);
		}
	}

	if (boolean_false == dump_all) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, cursor.next(&cursor, value_2));
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, cs_end_of_results, cursor.status);
		ion_external_sort_destroy_cursor(&cursor);
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, fclose(unsorted_file));

	if (NULL != sorted_file) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, fclose(sorted_file));
	}
}

void
test_macro_get_flag(
	planck_unit_test_t *tc
) {
	uint8_t bit_vector[10];

	int i;
	for (i = 0; i < 10; i++) {
		bit_vector[i] = 0; /* bit_vector[i] = 0000 0000 */
	}

	bit_vector[1] = 0xFF; /* 1111 1111 */
	bit_vector[9] = 0x23; /* 0010 0011 */

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
test_macro_set_flag(
	planck_unit_test_t *tc
) {
	uint8_t bit_vector[10];

	int i;
	for (i = 0; i < 10; i++) {
		bit_vector[i] = 0; /* bit_vector[i] = 0000 0000 */
	}

	ION_FMS_SET_FLAG(bit_vector, 8 * 9); /* bit_vector[9] = 0000 0001 */
	ION_FMS_SET_FLAG(bit_vector, 8 * 9 + 6); /* bit_vector[9] = 0100 0001 */

	for (; i < 9; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, bit_vector[i]);
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0x41, bit_vector[9]);
}

void
test_macro_clear_flag(
	planck_unit_test_t *tc
) {
	uint8_t bit_vector[10];

	int i;
	for (i = 0; i < 10; i++) {
		bit_vector[i] = 0xFF; /* bit_vector[i] = 1111 1111 */
	}

	ION_FMS_CLEAR_FLAG(bit_vector, 8 * 9); /* bit_vector[9] = 1111 1110 */
	ION_FMS_CLEAR_FLAG(bit_vector, 8 * 9 + 6); /* bit_vector[9] = 1011 1110 */

	for (; i < 9; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0xFF, bit_vector[i]);
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0xBE, bit_vector[9]);
}

void
test_flash_min_sort_required_memory(
	planck_unit_test_t *tc
) {
	FILE *unsorted_file = fopen(ion_external_sort_unsorted_fname, "w+b");

	PLANCK_UNIT_ASSERT_TRUE(tc, unsorted_file != NULL);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, ion_external_sort_write_uint16_random_data(unsorted_file, 1000));

	ion_external_sort_t			es;

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, ion_external_sort_init(&es, unsorted_file, NULL, ion_external_sort_uint16_comparator, sizeof(uint16_t), sizeof(uint16_t), 512, boolean_false, ION_FILE_SORT_FLASH_MINSORT));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 516 + 7, ion_external_sort_bytes_of_memory_required(&es, 0, boolean_false));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 516 + 9, ion_external_sort_bytes_of_memory_required(&es, 527, boolean_false));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 516 + 13, ion_external_sort_bytes_of_memory_required(&es, 529, boolean_false));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 516 + 13, ion_external_sort_bytes_of_memory_required(&es, 1000, boolean_false));

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 516 * 2 + 13, ion_external_sort_bytes_of_memory_required(&es, 1500, boolean_false));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 516 * 3 + 13, ion_external_sort_bytes_of_memory_required(&es, 2000, boolean_false));

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 516 * 2 + 7, ion_external_sort_bytes_of_memory_required(&es, 0, boolean_true));
}

void
test_flash_min_sort_small_sort_record_at_a_time_with_page_aligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 1000;
	uint32_t max_buffer_size = 0;
	ion_value_size_t value_size = sizeof(uint16_t);
	ion_boolean_e dump_all = boolean_false;
	ion_boolean_e sorted_pages = boolean_false;
	ion_sort_comparator_t comparator = ion_external_sort_uint16_comparator;

	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_uint16_random_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, NULL, comparator);

	write_func = ion_external_sort_write_uint16_sequential_data;
	ion_external_sort_sorted_value_func_t sorted_val_func = ion_external_sort_expected_uint16_sequential_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);
}

void
test_flash_min_sort_large_sort_record_at_a_time_with_page_aligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 10000;
	uint32_t max_buffer_size = 1500;
	ion_value_size_t value_size = sizeof(uint16_t);
	ion_boolean_e dump_all = boolean_false;
	ion_boolean_e sorted_pages = boolean_false;
	ion_sort_comparator_t comparator = ion_external_sort_uint16_comparator;

	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_uint16_random_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, NULL, comparator);

	write_func = ion_external_sort_write_uint16_sequential_data;
	ion_external_sort_sorted_value_func_t sorted_val_func = ion_external_sort_expected_uint16_sequential_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);
}

void
test_flash_min_sort_small_sort_record_at_a_time_with_page_unaligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 1000;
	uint32_t max_buffer_size = 0;
	ion_value_size_t value_size = 5;
	ion_boolean_e dump_all = boolean_false;
	ion_boolean_e sorted_pages = boolean_false;
	ion_sort_comparator_t comparator = ion_external_sort_5_char_str_comparator;

	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_5_char_str_random_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, NULL, comparator);

	ion_external_sort_sorted_value_func_t sorted_val_func = ion_external_sort_expected_5_char_str_sequential_data;
	write_func = ion_external_sort_write_5_char_str_sequential_data_descending;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);

	write_func = ion_external_sort_write_5_char_str_sequential_data_ascending;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);
}

void
test_flash_min_sort_large_sort_record_at_a_time_with_page_unaligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 10000;
	uint32_t max_buffer_size = 1500;
	ion_value_size_t value_size = 5;
	ion_boolean_e dump_all = boolean_false;
	ion_boolean_e sorted_pages = boolean_false;
	ion_sort_comparator_t comparator = ion_external_sort_5_char_str_comparator;

	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_5_char_str_random_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, NULL, comparator);

	ion_external_sort_sorted_value_func_t sorted_val_func = ion_external_sort_expected_5_char_str_sequential_data;
	write_func = ion_external_sort_write_5_char_str_sequential_data_descending;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);

	write_func = ion_external_sort_write_5_char_str_sequential_data_ascending;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);
}

void
test_flash_min_sort_small_sort_dump_to_file_with_page_aligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 1000;
	uint32_t max_buffer_size = 0;
	ion_value_size_t value_size = sizeof(uint16_t);
	ion_boolean_e dump_all = boolean_true;
	ion_boolean_e sorted_pages = boolean_false;
	ion_sort_comparator_t comparator = ion_external_sort_uint16_comparator;

	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_uint16_random_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, NULL, comparator);

	write_func = ion_external_sort_write_uint16_sequential_data;
	ion_external_sort_sorted_value_func_t sorted_val_func = ion_external_sort_expected_uint16_sequential_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);
}

void
test_flash_min_sort_large_sort_dump_to_file_with_page_aligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 10000;
	uint32_t max_buffer_size = 1500;
	ion_value_size_t value_size = sizeof(uint16_t);
	ion_boolean_e dump_all = boolean_true;
	ion_boolean_e sorted_pages = boolean_false;
	ion_sort_comparator_t comparator = ion_external_sort_uint16_comparator;

	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_uint16_random_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, NULL, comparator);

	write_func = ion_external_sort_write_uint16_sequential_data;
	ion_external_sort_sorted_value_func_t sorted_val_func = ion_external_sort_expected_uint16_sequential_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);
}

void
test_flash_min_sort_small_sort_dump_to_file_with_page_unaligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 1000;
	uint32_t max_buffer_size = 0;
	ion_value_size_t value_size = 5;
	ion_boolean_e dump_all = boolean_true;
	ion_boolean_e sorted_pages = boolean_false;
	ion_sort_comparator_t comparator = ion_external_sort_5_char_str_comparator;

	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_5_char_str_random_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, NULL, comparator);

	ion_external_sort_sorted_value_func_t sorted_val_func = ion_external_sort_expected_5_char_str_sequential_data;
	write_func = ion_external_sort_write_5_char_str_sequential_data_descending;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);

	write_func = ion_external_sort_write_5_char_str_sequential_data_ascending;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);

}

void
test_flash_min_sort_large_sort_dump_to_file_with_page_unaligned_data(
	planck_unit_test_t *tc
) {
	uint32_t num_records = 10000;
	uint32_t max_buffer_size = 1500;
	ion_value_size_t value_size = 5;
	ion_boolean_e dump_all = boolean_true;
	ion_boolean_e sorted_pages = boolean_false;
	ion_sort_comparator_t comparator = ion_external_sort_5_char_str_comparator;

	ion_external_sort_write_data_func_t write_func = ion_external_sort_write_5_char_str_random_data;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, NULL, comparator);

	ion_external_sort_sorted_value_func_t sorted_val_func = ion_external_sort_expected_5_char_str_sequential_data;
	write_func = ion_external_sort_write_5_char_str_sequential_data_descending;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);

	write_func = ion_external_sort_write_5_char_str_sequential_data_ascending;
	test_sort(tc, num_records, max_buffer_size, value_size, dump_all, sorted_pages, ION_FILE_SORT_FLASH_MINSORT, write_func, sorted_val_func, comparator);
}

planck_unit_suite_t *
file_sort_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_macro_get_flag);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_macro_set_flag);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_macro_clear_flag);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_required_memory);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_small_sort_record_at_a_time_with_page_aligned_data);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_large_sort_record_at_a_time_with_page_aligned_data);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_small_sort_record_at_a_time_with_page_unaligned_data);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_large_sort_record_at_a_time_with_page_unaligned_data);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_small_sort_dump_to_file_with_page_aligned_data);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_large_sort_dump_to_file_with_page_aligned_data);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_small_sort_dump_to_file_with_page_unaligned_data);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flash_min_sort_large_sort_dump_to_file_with_page_unaligned_data);

	return suite;
}

void
runalltests_file_sort(
) {
	planck_unit_suite_t *suite = file_sort_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
