#include "test_in_memory_sort.h"

void
test_(
	planck_unit_test_t *tc
) {
	PLANCK_UNIT_ASSERT_TRUE(tc, 1);
}

planck_unit_suite_t *
in_memory_sort_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_);

	return suite;
}

void
runalltests_in_memory_sort(
) {
	planck_unit_suite_t *suite = in_memory_sort_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
