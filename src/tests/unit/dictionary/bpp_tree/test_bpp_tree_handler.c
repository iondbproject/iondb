#include "test_bpp_tree_handler.h"

void
run_bpptreehandler_generic_test_set_1(
	planck_unit_test_t *tc
) {
	generic_test_t test;

	int					keyvals[]	= { 5, -10, -5 };
	ion_key_t			keys[]		= { &keyvals[0], &keyvals[1], &keyvals[2] };
	int					length		= sizeof(keys) / sizeof(ion_key_t);
	ion_result_count_t	count;
	ion_result_count_t	counts[length];
	int					k;

	int i;

	for (i = 0; i < length; i++) {
		counts[i] = 0;
	}

	init_generic_dictionary_test(&test, bpptree_init, key_type_numeric_signed, sizeof(int), sizeof(int), -1	/* Dictionary size, for now is unbounded. */
	);

	dictionary_test_init(&test, tc);

	dictionary_test_insert_get(&test, 10000, keys, counts, length, tc);

	dictionary_test_insert_get_edge_cases(&test, keys, counts, length, tc);

	int to_delete[]		= { 7, -9, 32, 1000001 };
	int to_delete_len	= (int) (sizeof(to_delete) / sizeof(int));

	for (i = 0; i < to_delete_len; i++) {
		k = get_count_index_by_key((&(to_delete[i])), keys, length, &(test.dictionary));

		if (k != -1) {
			count = counts[k];
		}
		else {
			count = 0;
		}

		dictionary_test_delete(&test, (ion_key_t) (&(to_delete[i])), count, tc);

		if (k != -1) {
			counts[k] = 0;
		}
	}

	int update_key;
	int update_value;

	update_key		= 1;
	update_value	= -12;

	k				= get_count_index_by_key((&update_key), keys, length, &(test.dictionary));

	if (k != -1) {
		count = counts[k];
	}
	else {
		count = 0;
	}

	dictionary_test_update(&test, (ion_key_t) (&update_key), (ion_value_t) (&update_value), count, tc);

	update_key		= 1;
	update_value	= 12;
	k				= get_count_index_by_key((&update_key), keys, length, &(test.dictionary));

	if (k != -1) {
		count = counts[k];
	}
	else {
		count = 0;
	}

	dictionary_test_update(&test, (ion_key_t) (&update_key), (ion_value_t) (&update_value), count, tc);

	update_key		= 12;
	update_value	= 1;
	k				= get_count_index_by_key((&update_key), keys, length, &(test.dictionary));

	if (k != -1) {
		count = counts[k];
	}
	else {
		count = 0;
	}

	dictionary_test_update(&test, (ion_key_t) (&update_key), (ion_value_t) (&update_value), count, tc);

	dictionary_insert(&test.dictionary, IONIZE(5, int), IONIZE(3, int));
	dictionary_insert(&test.dictionary, IONIZE(5, int), IONIZE(5, int));
	dictionary_insert(&test.dictionary, IONIZE(5, int), IONIZE(7, int));

	dictionary_insert(&test.dictionary, IONIZE(-5, int), IONIZE(14, int));
	dictionary_insert(&test.dictionary, IONIZE(-7, int), IONIZE(6, int));
	dictionary_insert(&test.dictionary, IONIZE(-10, int), IONIZE(23, int));
	dictionary_insert(&test.dictionary, IONIZE(-205, int), IONIZE(9, int));

	dictionary_test_equality(&test, IONIZE(5, int), tc);

	dictionary_test_equality(&test, IONIZE(-10, int), tc);

	dictionary_test_range(&test, IONIZE(5, int), IONIZE(3777, int), tc);

	dictionary_test_range(&test, IONIZE(-5, int), IONIZE(3777, int), tc);

	dictionary_test_all_records(&test, 10006, tc);

	dictionary_test_open_close(&test, tc);

	cleanup_generic_dictionary_test(&test);
}

planck_unit_suite_t *
bpptreehandler_get_suite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	planck_unit_add_to_suite(suite, run_bpptreehandler_generic_test_set_1);

	return suite;
}

void
run_all_tests_bpptreehandler(
) {
	planck_unit_suite_t *suite = bpptreehandler_get_suite();

	planck_unit_run_suite(suite);

	planck_unit_destroy_suite(suite);
}
