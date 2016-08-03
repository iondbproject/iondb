#include "test_external_sort.h"

#define ION_EXTERNAL_SORT_SEED 0x8E01483F
#define ION_EXTERNAL_SORT_PAGE_SIZE 512
#define ION_EXTERNAL_SORT_BUFFER_SIZE 512

const char ion_external_sort_uint16_fname[] = "uint16.dat";

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
//		printf("%d\n", random);

		if (0 == fwrite(&random, sizeof(uint16_t), 1, unsorted_file)) {
			return err_file_read_error;
		}
	}

	return err_ok;
}

//ion_err_t
//ion_external_sort_check_uint16_data(
//	char *file_name
//) {
//	FILE *sorted_file = fopen(file_name, "rb");
//
//
//}

void
test_1_page_region_record_at_a_time(
	planck_unit_test_t *tc
) {
	uint32_t num_values = 1000;

	FILE *unsorted_file = fopen(ion_external_sort_uint16_fname, "w+b");
	PLANCK_UNIT_ASSERT_TRUE(tc, unsorted_file != NULL);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, ion_external_sort_write_uint16_data(unsorted_file, num_values));

	ion_external_sort_t es;
	ion_external_sort_cursor_t cursor;
	uint8_t buffer[8];

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok,
	ion_external_sort_init(
		&es,
		unsorted_file,
		NULL,
		ion_external_sort_uint16_comparator,
		sizeof(uint16_t),
		sizeof(uint16_t),
		ION_EXTERNAL_SORT_PAGE_SIZE,
		boolean_false,
		ION_FILE_SORT_FLASH_MINSORT
	));

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok,
	ion_external_sort_init_cursor(
		&es,
		&cursor,
		buffer,
		8
	));

	uint16_t value_1;
	uint16_t value_2;

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, cursor.next(&cursor, &value_1));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, cs_cursor_active, cursor.status);

	uint32_t i;
	for (i = 1; i < num_values; i++) {
		if (i == 999) {
			printf("f");
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, cursor.next(&cursor, &value_2));
		if (value_2 == 5365) {
//			printf("f");
		}
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, cs_cursor_active, cursor.status);

		ion_comparison_e comparison_return_value = ion_external_sort_uint16_comparator(NULL, &value_1, &value_2);
		PLANCK_UNIT_ASSERT_TRUE(tc, less_than == comparison_return_value || equal == comparison_return_value);
		value_1 = value_2;
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, cursor.next(&cursor, &value_2));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, cs_end_of_results, cursor.status);

	ion_external_sort_destroy_cursor(&cursor);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, fclose(unsorted_file));
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

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_1_page_region_record_at_a_time);
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
