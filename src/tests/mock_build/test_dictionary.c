/*
 * dictionary.c
 *
 *  Created on: Jul 15, 2014
 *	  Author: workstation
*/

#include "test_dictionary.h"

void
test_mock(
	planck_unit_test_t *tc
) {
	key_one = &(int) {
		1
	};
	key_two = &(int) {
		1
	};

	PLANCK_UNIT_ASSERT_TRUE(tc, IS_EQUAL == dictionary_compare_unsigned_value(1));
}

planck_unit_suite_t *
dictionary_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_mock);

	return suite;
}

void
runalltests_dictionary(
) {
	planck_unit_suite_t *suite = dictionary_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
