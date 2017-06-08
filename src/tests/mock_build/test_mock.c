/*
 * dictionary.c
 *
 *  Created on: Jul 15, 2014
 *	  Author: workstation
*/

#include "test_mock.h"
#include "../../dictionary/dictionary.c"
#include "../planckunit/src/planck_unit.c"

void
test_mock(
	planck_unit_test_t *tc
) {
	ion_key_t	key_one;
	ion_key_t	key_two;

	key_one = &(int) {
		1
	};
	key_two = &(int) {
		1
	};

	PLANCK_UNIT_ASSERT_TRUE(tc, IS_EQUAL == dictionary_compare_unsigned_value(key_one, key_two, sizeof(int)));
}

planck_unit_suite_t *
mock_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_mock);

	return suite;
}

void
runalltests_mock(
) {
	planck_unit_suite_t *suite = mock_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
