/******************************************************************************/
/**
@file		test_bpp_tree_handler.c
@author		IonDB Project
@brief		Entry point for bpp tree tests.
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#include "test_bpp_tree_handler.h"

void
run_bpptreehandler_generic_test_set_1(
	planck_unit_test_t *tc
) {
	ion_generic_test_t test;

	int					keyvals[100];
	ion_key_t			keys[100];
	int					length = sizeof(keys) / sizeof(ion_key_t);
	ion_result_count_t	count;
	ion_result_count_t	counts[length];
	int					k;

	int i;

	for (i = 0; i < length; i++) {
		keyvals[i]	= i;
		keys[i]		= &keyvals[i];
		counts[i]	= 0;
	}

	init_generic_dictionary_test(&test, bpptree_init, key_type_numeric_signed, sizeof(int), sizeof(int), -1	/* Dictionary size, for now is unbounded. */
	);

	dictionary_test_init(&test, tc);

	dictionary_test_insert_get(&test, 100 /* This used to be 10,000 */, keys, counts, length, tc);

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

		dictionary_test_delete(&test, (&(to_delete[i])), count, tc);

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

	dictionary_test_update(&test, (&update_key), (&update_value), count, tc);

	update_key		= 1;
	update_value	= 12;
	k				= get_count_index_by_key((&update_key), keys, length, &(test.dictionary));

	if (k != -1) {
		count = counts[k];
	}
	else {
		count = 0;
	}

	dictionary_test_update(&test, (&update_key), (&update_value), count, tc);

	update_key		= 12;
	update_value	= 1;
	k				= get_count_index_by_key((&update_key), keys, length, &(test.dictionary));

	if (k != -1) {
		count = counts[k];
	}
	else {
		count = 0;
	}

	dictionary_test_update(&test, (&update_key), (&update_value), count, tc);

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

	dictionary_test_all_records(&test, 106, tc);

	dictionary_test_open_close(&test, tc);

	cleanup_generic_dictionary_test(&test);
}

planck_unit_suite_t *
bpptreehandler_get_suite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, run_bpptreehandler_generic_test_set_1);

	return suite;
}

void
run_all_tests_bpptreehandler(
) {
	planck_unit_suite_t *suite = bpptreehandler_get_suite();

	planck_unit_run_suite(suite);

	planck_unit_destroy_suite(suite);
}
