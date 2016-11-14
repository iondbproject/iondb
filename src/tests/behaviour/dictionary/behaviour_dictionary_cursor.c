#include "../../../dictionary/dictionary_types.h"
#include "behaviour_dictionary.h"
#include "behaviour_dictionary_cursor.h"

/**
@brief	This function performs a find of the type specified on the predictate in a dictionary.
*/
void
test_bhdct_find(
	planck_unit_test_t	*tc,
	ion_dictionary_t	*dict,
	ion_predicate_t		*predicate,
	int					expected_result_count
) {
	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(dict, predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(dict->instance->record.key_size);
	ion_record.value	= malloc(dict->instance->record.value_size);

	int					result_count = 0;
	ion_cursor_status_t cursor_status;

	while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, NEUTRALIZE(ion_record.key, int) * 2, NEUTRALIZE(ion_record.value, int));
		result_count++;
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_result_count, result_count);

	/* Clean-up everything by removing the cursor. Also free the ion_record space we allocated. */
	cursor->destroy(&cursor);
	free(ion_record.key);
	free(ion_record.value);
}

/**
@brief	This function tests an equality find on a dictionary with a single record in it.
*/
void
test_bhdct_find_equality_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;
	ion_predicate_t				predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(55, int));

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(55, int), IONIZE(55 * 2, int), boolean_true);

	test_bhdct_find(tc, &dict, &predicate, 1);
	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an equality find on a dictionary with many records in it.
*/
void
test_bhdct_find_equality_in_many(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;
	ion_predicate_t				predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(55, int));

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(55, int), IONIZE(55 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(35, int), IONIZE(35 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(-5, int), IONIZE(-5 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(99, int), IONIZE(99 * 2, int), boolean_true);

	test_bhdct_find(tc, &dict, &predicate, 1);
	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an equality find on a dictionary with many records in it with duplicate keys of the queried value.
*/
void
test_bhdct_find_equality_duplicate_key(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;
	ion_predicate_t				predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(35, int));

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(35, int), IONIZE(35 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(35, int), IONIZE(35 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(35, int), IONIZE(35 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(-5, int), IONIZE(-5 * 2, int), boolean_true);

	test_bhdct_find(tc, &dict, &predicate, 3);
	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an equality find on a dictionary with no keys of the queried value.
*/
void
test_bhdct_find_equality_no_result(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;
	ion_predicate_t				predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(60, int));

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(35, int), IONIZE(35 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(-5, int), IONIZE(-5 * 2, int), boolean_true);

	test_bhdct_find(tc, &dict, &predicate, 0);
	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an equality find on an empty dictionary.
*/
void
test_bhdct_find_equality_empty_dict(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;
	ion_predicate_t				predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(60, int));

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	test_bhdct_find(tc, &dict, &predicate, 0);
	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests a range find on a dictionary with a single record in it.
*/
void
test_bhdct_find_range_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;
	ion_predicate_t				predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(25, int), IONIZE(100, int));

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(55, int), IONIZE(55 * 2, int), boolean_true);

	test_bhdct_find(tc, &dict, &predicate, 1);
	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests a range find on a dictionary with many records in it.
*/
void
test_bhdct_find_range_in_many(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;
	ion_predicate_t				predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(25, int), IONIZE(100, int));

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(55, int), IONIZE(55 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(35, int), IONIZE(35 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(-5, int), IONIZE(-5 * 2, int), boolean_true);

	test_bhdct_find(tc, &dict, &predicate, 2);
	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an equality find on a dictionary with duplicate keys in the queried range.
*/
void
test_bhdct_find_range_duplicate_key(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;
	ion_predicate_t				predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(25, int), IONIZE(100, int));

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(35, int), IONIZE(35 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(55, int), IONIZE(55 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(55, int), IONIZE(55 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(-5, int), IONIZE(-5 * 2, int), boolean_true);

	test_bhdct_find(tc, &dict, &predicate, 3);
	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an equality find on a dictionary with no keys in the queried range.
*/
void
test_bhdct_find_range_no_result(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;
	ion_predicate_t				predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(0, int), IONIZE(10, int));

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(35, int), IONIZE(35 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(-5, int), IONIZE(-5 * 2, int), boolean_true);

	test_bhdct_find(tc, &dict, &predicate, 0);
	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an equality find on an empty dictionary.
*/
void
test_bhdct_find_range_empty_dict(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;/* handler binds functions of the dictionary to correct implemention */
	ion_dictionary_t			dict;
	ion_predicate_t				predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(0, int), IONIZE(10, int));

	bhdct_setup(tc, &handler, &dict, ion_fill_none);
	test_bhdct_find(tc, &dict, &predicate, 0);
	bhdct_takedown(tc, &dict);
}

void
bhdct_run_cursor_tests(
	ion_handler_initializer_t	init_fcn,
	ion_dictionary_size_t		dictionary_size,
	uint32_t					test_classes
) {
	bhdct_context.init_fcn			= init_fcn;
	bhdct_context.dictionary_size	= dictionary_size;
	bhdct_context.test_classes		= test_classes;

	if (bhdct_context.test_classes & ION_BHDCT_INT_INT) {
		planck_unit_suite_t *suite = planck_unit_new_suite();

		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_equality_single);
		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_equality_in_many);
		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_equality_no_result);
		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_equality_empty_dict);

		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_range_single);
		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_range_in_many);
		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_range_no_result);
		/*PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_range_empty_dict);*/

		planck_unit_run_suite(suite);
		planck_unit_destroy_suite(suite);
	}

	if (bhdct_context.test_classes & ION_BHDCT_STRING_INT) {
		planck_unit_suite_t *suite = planck_unit_new_suite();

		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_equality_single);
		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_equality_in_many);
		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_equality_no_result);
		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_equality_empty_dict);

		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_range_single);
		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_range_in_many);
		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_range_no_result);
		/*PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_range_empty_dict);*/

		planck_unit_run_suite(suite);
		planck_unit_destroy_suite(suite);
	}

	if (bhdct_context.test_classes & ION_BHDCT_DUPLICATES) {
		planck_unit_suite_t *suite = planck_unit_new_suite();

		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_range_duplicate_key);
		PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_find_equality_duplicate_key);

		planck_unit_run_suite(suite);
		planck_unit_destroy_suite(suite);
	}
}
