/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Unit test for skiplist handler interface.
*/
/******************************************************************************/

#include "test_skip_list_handler.h"

/**
@brief	  Helper function that constructs a sample dictionary based on the
			given parameters.

@param	  dictionary
				Dictionary to initialize
@param	  handler
				Handler to bind
@param	  record
				Secondary information within dictionary
@param	  key_type
				Type of key used
@param	  size
				Size of dictionary
*/
void
create_test_dictionary(
	ion_dictionary_t			*dictionary,
	ion_dictionary_handler_t	*handler,
	ion_record_info_t			*record,
	ion_key_type_t				key_type,
	int							size,
	int							num_elements
) {
	sldict_init(handler);

	dictionary_create(handler, dictionary, 1, key_type, record->key_size, record->value_size, size);

	/* Populate dictionary */
	int		half_elements = num_elements / 2;
	char	*value = "DATA";
	int		i, j, num_duplicates = 0;

	/* First insert one of each element, up to half... */
	for (i = 0; i < half_elements; i++) {
		dictionary_insert(dictionary, &i, value);
	}

	/* Continue inserting, this time with an increasing amount of duplicates */
	for (; i < num_elements; i++) {
		for (j = 0; j < num_duplicates; j++) {
			dictionary_insert(dictionary, &i, value);
		}

		/* Each time we increment the key, add one more duplicate */
		num_duplicates++;
	}
}

/**
@brief	  Helper function to create a dictionary instance using standard condition
			variables.

@param	  dictionary
				Dictionary to initialize
@param	  handler
				Handler to bind
*/
void
create_test_dictionary_std_conditions(
	ion_dictionary_t			*dictionary,
	ion_dictionary_handler_t	*handler
) {
	/* This means keysize 4 (on a desktop platform) and valuesize 10 */
	ion_record_info_t	record			= { sizeof(int), 10 };
	ion_key_type_t		key_type		= key_type_numeric_signed;
	int					size			= 7;
	int					num_elements	= 50;

	create_test_dictionary(dictionary, handler, &record, key_type, size, num_elements);
}

/**
@brief	  Tests the creation of a handler and verifies all function pointers
			have been correctly bound.

@param	  tc
				CuTest dependency
*/
void
test_dictionary_handler_binding(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	ion_dictionary_handler_t handler;

	sldict_init(&handler);

	PLANCK_UNIT_ASSERT_TRUE(tc, handler.insert == &sldict_insert);
	PLANCK_UNIT_ASSERT_TRUE(tc, handler.create_dictionary == &sldict_create_dictionary);
	PLANCK_UNIT_ASSERT_TRUE(tc, handler.update == &sldict_update);
	PLANCK_UNIT_ASSERT_TRUE(tc, handler.remove == &sldict_delete);
	PLANCK_UNIT_ASSERT_TRUE(tc, handler.delete_dictionary == &sldict_delete_dictionary);
}

/**
@brief	  Tests the creation of a dictionary instance and verifies all properties
			have been correctly initialized.

@param	  tc
				CuTest dependency
*/
void
test_dictionary_creation(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	ion_dictionary_t			dict;
	ion_dictionary_handler_t	handler;
	ion_record_info_t			record			= { sizeof(int), 10 };
	ion_key_type_t				key_type		= key_type_numeric_signed;
	int							size			= 50;
	int							num_elements	= 25;

	create_test_dictionary(&dict, &handler, &record, key_type, size, num_elements);

	ion_skiplist_t *skiplist = (ion_skiplist_t *) dict.instance;

	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->key_type == key_type_numeric_signed);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare == dictionary_compare_signed_value);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->record.key_size == sizeof(int));
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->record.value_size == 10);
	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist != NULL);
	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist->head != NULL);
	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist->maxheight == 50);
	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist->pden == 4);
	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist->pnum == 1);
	PLANCK_UNIT_ASSERT_TRUE(tc, &skiplist->super != NULL);

	dictionary_delete_dictionary(&dict);
}

/**
@brief	  Tests an equality cursor on the std conditions skiplist. Assertion
			is that the cursor will return err_ok.

@param	  tc
				CuTest dependency
*/
void
test_slhandler_cursor_equality(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	ion_dictionary_t			dict;
	ion_dictionary_handler_t	handler;

	create_test_dictionary_std_conditions(&dict, &handler);

	ion_dict_cursor_t	*cursor;
	ion_predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(33, int));

	ion_err_t status = dictionary_find(&dict, &predicate, &cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status);

	cursor->destroy(&cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);

	dictionary_delete_dictionary(&dict);
}

/**
@brief	  Tests an equality cursor on the std conditions skiplist. The
			assertion is that the values returned will satisfy the predicate.

@param	  tc
				CuTest dependency
*/
void
test_slhandler_cursor_equality_with_results(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	ion_dictionary_t			dict;
	ion_dictionary_handler_t	handler;

	create_test_dictionary_std_conditions(&dict, &handler);

	ion_dict_cursor_t	*cursor;
	ion_predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(26, int));

	ion_err_t status = dictionary_find(&dict, &predicate, &cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status);
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;

	record.key		= malloc(dict.instance->record.key_size);
	record.value	= malloc(dict.instance->record.value_size);

	ion_cursor_status_t c_status = cursor->next(cursor, &record);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active == c_status);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(26, int), dict.instance->record.key_size) == 0);
	PLANCK_UNIT_ASSERT_TRUE(tc, memcmp(record.value, (char *) { "DATA" }, dict.instance->record.value_size) == 0);

	while (c_status != cs_end_of_results) {
		PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(26, int), dict.instance->record.key_size) == 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, memcmp(record.value, (char *) { "DATA" }, dict.instance->record.value_size) == 0);
		c_status = cursor->next(cursor, &record);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->status);

	cursor->destroy(&cursor);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);

	free(record.key);
	free(record.value);
	dictionary_delete_dictionary(&dict);
}

/**
@brief	  Tests a range cursor on the std conditions skiplist. Assertion
			is that the cursor will return err_ok.

@param	  tc
				CuTest dependency
*/
void
test_slhandler_cursor_range(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	ion_dictionary_t			dict;
	ion_dictionary_handler_t	handler;

	create_test_dictionary_std_conditions(&dict, &handler);

	ion_dict_cursor_t	*cursor;
	ion_predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(15, int), IONIZE(60, int));

	ion_err_t status = dictionary_find(&dict, &predicate, &cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status);

	cursor->destroy(&cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);

	dictionary_delete_dictionary(&dict);
}

/**
@brief	  Tests a range cursor on the std conditions skiplist. The
			assertion is that the values returned will satisfy the predicate.

@param	  tc
				CuTest dependency
*/
void
test_slhandler_cursor_range_with_results(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	ion_dictionary_t			dict;
	ion_dictionary_handler_t	handler;

	create_test_dictionary_std_conditions(&dict, &handler);

	ion_dict_cursor_t	*cursor;
	ion_predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(5, int), IONIZE(78, int));

	ion_err_t status = dictionary_find(&dict, &predicate, &cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status);
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;

	record.key		= malloc(dict.instance->record.key_size);
	record.value	= malloc(dict.instance->record.value_size);

	ion_cursor_status_t c_status = cursor->next(cursor, &record);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active == c_status);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(5, int), dict.instance->record.key_size) >= 0);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(78, int), dict.instance->record.key_size) <= 0);
	PLANCK_UNIT_ASSERT_TRUE(tc, memcmp(record.value, (char *) { "DATA" }, dict.instance->record.value_size) == 0);

	while (c_status != cs_end_of_results) {
		PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(5, int), dict.instance->record.key_size) >= 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(78, int), dict.instance->record.key_size) <= 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, memcmp(record.value, (char *) { "DATA" }, dict.instance->record.value_size) == 0);
		c_status = cursor->next(cursor, &record);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->status);

	cursor->destroy(&cursor);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);

	free(record.key);
	free(record.value);
	dictionary_delete_dictionary(&dict);
}

/**
@brief		Tests a range cursor on the std conditions skiplist. The edge
			case we're testing is when the lower bound is smaller than all existing
			data, but there is actually valid data within the range.
@details	An example of this is if you have [3,3,4,4] inserted as keys, and then you
			ask for the range query with lower = 2 and upper 3. We should get [3,3] back
			as the result of the query.

@param	  tc
				CuTest dependency
*/
void
test_slhandler_cursor_range_lower_missing(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	ion_dictionary_t			dict;
	ion_dictionary_handler_t	handler;

	create_test_dictionary_std_conditions(&dict, &handler);

	ion_dict_cursor_t	*cursor;
	ion_predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(-50, int), IONIZE(50, int));

	ion_err_t status = dictionary_find(&dict, &predicate, &cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status);
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;

	record.key		= malloc(dict.instance->record.key_size);
	record.value	= malloc(dict.instance->record.value_size);

	ion_cursor_status_t c_status = cursor->next(cursor, &record);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active == c_status);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(-50, int), dict.instance->record.key_size) >= 0);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(50, int), dict.instance->record.key_size) <= 0);
	PLANCK_UNIT_ASSERT_TRUE(tc, memcmp(record.value, (char *) { "DATA" }, dict.instance->record.value_size) == 0);

	while (c_status != cs_end_of_results) {
		PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(-50, int), dict.instance->record.key_size) >= 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(50, int), dict.instance->record.key_size) <= 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, memcmp(record.value, (char *) { "DATA" }, dict.instance->record.value_size) == 0);
		c_status = cursor->next(cursor, &record);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->status);

	cursor->destroy(&cursor);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);

	free(record.key);
	free(record.value);
	dictionary_delete_dictionary(&dict);
}

/**
@brief		Tests a range cursor on the std conditions skiplist. We use an exact data set,
			and then assert that we saw everything that was expected to be seen. This is the final
			line of defense against a broken range query.
@param	  tc
				CuTest dependency
*/
void
test_slhandler_cursor_range_exact_results(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	ion_dictionary_t			dict;
	ion_dictionary_handler_t	handler;

	create_test_dictionary_std_conditions(&dict, &handler);

	int extra_keys[]	= { 503, 504, 504, 504, 509, 542 };
	int num_extra		= sizeof(extra_keys) / sizeof(int);

	int i;

	for (i = 0; i < num_extra; i++) {
		ion_status_t status = dictionary_insert(&dict, &extra_keys[i], "test");

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	ion_dict_cursor_t	*cursor;
	ion_predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(500, int), IONIZE(600, int));

	ion_err_t status = dictionary_find(&dict, &predicate, &cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status);
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;

	record.key		= malloc(dict.instance->record.key_size);
	record.value	= malloc(dict.instance->record.value_size);

	ion_cursor_status_t c_status = cursor->next(cursor, &record);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active == c_status);

	int key_idx = 0;

	while (c_status != cs_end_of_results) {
		int expected_key = extra_keys[key_idx];

		PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, &expected_key, dict.instance->record.key_size) == 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, memcmp(record.value, (char *) { "test" }, dict.instance->record.value_size) == 0);
		c_status = cursor->next(cursor, &record);
		key_idx++;
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->status);
	/* This means we have seen all that we expected to see */
	PLANCK_UNIT_ASSERT_TRUE(tc, key_idx == num_extra);

	cursor->destroy(&cursor);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);

	free(record.key);
	free(record.value);
	dictionary_delete_dictionary(&dict);
}

/**
@brief	  Creates the suite to test using CuTest.
@return	 Pointer to a CuTest suite.
*/
planck_unit_suite_t *
skiplist_handler_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	/* Creation test */
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_dictionary_handler_binding);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_dictionary_creation);

	/* Cursor Equality test */
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_slhandler_cursor_equality);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_slhandler_cursor_equality_with_results);

	/* Cursor Range test */
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_slhandler_cursor_range);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_slhandler_cursor_range_with_results);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_slhandler_cursor_range_lower_missing);
/*	PLANCK_UNIT_ADD_TO_SUITE(suite, test_slhandler_cursor_range_exact_results); */

	return suite;
}

/**
@brief	  Runs all skiplist related test and outputs the result.
*/
void
runalltests_skiplist_handler(
) {
	/* CuString	*output	= CuStringNew(); */
	planck_unit_suite_t *suite = skiplist_handler_getsuite();

	planck_unit_run_suite(suite);
	/* CuSuiteSummary(suite, output); */
	/* CuSuiteDetails(suite, output); */
	/* printf("----\nSkiplist Handler Tests:\n%s\n", output->buffer); */

	planck_unit_destroy_suite(suite);
	/* CuSuiteDelete(suite); */
	/* CuStringDelete(output); */
}
