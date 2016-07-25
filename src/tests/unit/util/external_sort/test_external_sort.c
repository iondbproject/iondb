#include "test_external_sort.h"

void
test_(
	planck_unit_test_t *tc
) {
	PLANCK_UNIT_ASSERT_TRUE(tc, 1);
}

/* void */
/* test_macro_get_flag( */
/*	planck_unit_test_t *tc */
/* ) { */
/*	uint8_t bit_vector[10]; */
/*  */
/*	int i; */
/*	for (i = 0; i < 10; i++) { */
/*		bit_vector[i] = 0; // 0000 0000 */
/*	} */
/*  */
/*	bit_vector[1] = 0xFF; // 1111 1111 */
/*	bit_vector[9] = 0x23; // 0010 0011 */
/*  */
/*	for (i = 0; i < 8 * 1; i++) { */
/*		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, ION_FLASH_MIN_SORT_GET_FLAG(bit_vector, i)); */
/*	} */
/*  */
/*	for (; i < 8 * 2; i++) { */
/*		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, ION_FLASH_MIN_SORT_GET_FLAG(bit_vector, i)); */
/*	} */
/*  */
/*	for (; i < 8 * 9; i++) { */
/*		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, ION_FLASH_MIN_SORT_GET_FLAG(bit_vector, i)); */
/*	} */
/*  */
/*	for (; i < 8 * 9 + 2; i++) { */
/*		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, ION_FLASH_MIN_SORT_GET_FLAG(bit_vector, i)); */
/*	} */
/*  */
/*	for (; i < 8 * 9 + 5; i++) { */
/*		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, ION_FLASH_MIN_SORT_GET_FLAG(bit_vector, i)); */
/*	} */
/*  */
/*	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, ION_FLASH_MIN_SORT_GET_FLAG(bit_vector, i)); */
/*	i++; */
/*  */
/*	for (; i < 8 * 10; i++) { */
/*		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, ION_FLASH_MIN_SORT_GET_FLAG(bit_vector, i)); */
/*	} */
/* } */
/*  */
/* void */
/* test_macro_toggle_flag( */
/*	planck_unit_test_t *tc */
/* ) { */
/*	uint8_t bit_vector[10]; */
/*  */
/*	int i; */
/*	for (i = 0; i < 10; i++) { */
/*		bit_vector[i] = 0; // 0000 0000 */
/*	} */
/*  */
/*	ION_FLASH_MIN_SORT_TOGGLE_FLAG(bit_vector, 8 * 9); // bit_vector[9] = 0000 0001 */
/*	ION_FLASH_MIN_SORT_TOGGLE_FLAG(bit_vector, 8 * 9 + 6); // bit_vector[9] = 0100 0001 */
/*  */
/*	for (; i < 9; i++) { */
/*		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, bit_vector[i]); */
/*	} */
/*  */
/*	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0x41, bit_vector[9]); */
/*  */
/*	ION_FLASH_MIN_SORT_TOGGLE_FLAG(bit_vector, 8 * 9 + 6); // bit_vector[9] = 0000 0001 */
/*	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0x01, bit_vector[9]); */
/* } */

planck_unit_suite_t *
file_sort_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_);

	return suite;
}

void
runalltests_file_sort(
) {
	planck_unit_suite_t *suite = file_sort_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
