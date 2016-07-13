#include "test_iinq.h"

void
iinq_test_empty(
	planck_unit_test_t *tc
) {
	PLANCK_UNIT_ASSERT_TRUE(tc, 1);
}

planck_unit_suite_t *
iinq_get_suite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_empty);

	return suite;
}

void
run_all_tests_iinq(
) {
	planck_unit_suite_t *suite = iinq_get_suite();

	planck_unit_run_suite(suite);

	planck_unit_destroy_suite(suite);
}
