/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Unit tests for skiplist handler interface.
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
create_test_collection(
	dictionary_t			*dictionary,
	dictionary_handler_t	*handler,
	record_info_t			*record,
	key_type_t				key_type,
	int						size,
	int						num_elements
) {
	sldict_init(handler);

	dictionary_create(handler, dictionary, 1, key_type, record->key_size, record->value_size, size);

	/* Populate dictionary */
	int		half_elements = num_elements / 2;
	char	*value = "DATA";
	int		i, j;

	/* First insert one of each element, up to half... */
	for (i = 0; i < half_elements; i++) {
		dictionary_insert(dictionary, (ion_key_t) &i, (ion_value_t) value);
	}

	/* Continue inserting, this time with an increasing amount of duplicates */
	for (; i < num_elements; i++) {
		for (j = half_elements; j < i; j++) {
			dictionary_insert(dictionary, (ion_key_t) &i, (ion_value_t) value);
		}
	}
}

/**
@brief	  Helper function to create a collection using standard condition
			variables.

@param	  dictionary
				Dictionary to initialize
@param	  handler
				Handler to bind
 */
void
create_test_collection_std_conditions(
	dictionary_t			*dictionary,
	dictionary_handler_t	*handler
) {
	/* This means keysize 4 (on a desktop platform) and valuesize 10 */
	record_info_t	record			= { sizeof(int), 10 };
	key_type_t		key_type		= key_type_numeric_signed;
	int				size			= 7;
	int				num_elements	= 100;

	create_test_collection(dictionary, handler, &record, key_type, size, num_elements);
}

/**
@brief	  Tests the creation of a handler and verifies all function pointers
			have been correctly bound.

@param	  tc
				CuTest dependency
 */
void
test_collection_handler_binding(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	dictionary_handler_t handler;

	sldict_init(&handler);

	PLANCK_UNIT_ASSERT_TRUE(tc, handler.insert == &sldict_insert);
	PLANCK_UNIT_ASSERT_TRUE(tc, handler.create_dictionary == &sldict_create_dictionary);
	PLANCK_UNIT_ASSERT_TRUE(tc, handler.update == &sldict_update);
	PLANCK_UNIT_ASSERT_TRUE(tc, handler.remove == &sldict_delete);
	PLANCK_UNIT_ASSERT_TRUE(tc, handler.delete_dictionary == &sldict_delete_dictionary);
}

/**
@brief	  Tests the creation of a collection and verifies all properties
			have been correctly initialized.

@param	  tc
				CuTest dependency
 */
void
test_collection_creation(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	dictionary_t			dict;
	dictionary_handler_t	handler;
	record_info_t			record			= { sizeof(int), 10 };
	key_type_t				key_type		= key_type_numeric_signed;
	int						size			= 50;
	int						num_elements	= 25;

	create_test_collection(&dict, &handler, &record, key_type, size, num_elements);

	skiplist_t *skiplist = (skiplist_t *) dict.instance;

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

	dictionary_t			dict;
	dictionary_handler_t	handler;

	create_test_collection_std_conditions(&dict, &handler);

	dict_cursor_t	*cursor;
	predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(33, int));

	err_t status = dictionary_find(&dict, &predicate, &cursor);

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

	dictionary_t			dict;
	dictionary_handler_t	handler;

	create_test_collection_std_conditions(&dict, &handler);

	dict_cursor_t	*cursor;
	predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(56, int));

	err_t status = dictionary_find(&dict, &predicate, &cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status);
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;

	record.key		= malloc(dict.instance->record.key_size);
	record.value	= malloc(dict.instance->record.value_size);

	cursor_status_t c_status = cursor->next(cursor, &record);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active == c_status);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(56, int), dict.instance->record.key_size) == 0);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.value, (ion_value_t) (char *) { "DATA" }, dict.instance->record.value_size) == 0);

	while (cursor->next(cursor, &record) != cs_end_of_results) {
		PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(56, int), dict.instance->record.key_size) == 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.value, (ion_value_t) (char *) { "DATA" }, dict.instance->record.value_size) == 0);
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

	dictionary_t			dict;
	dictionary_handler_t	handler;

	create_test_collection_std_conditions(&dict, &handler);

	dict_cursor_t	*cursor;
	predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(15, int), IONIZE(60, int));

	err_t status = dictionary_find(&dict, &predicate, &cursor);

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

	dictionary_t			dict;
	dictionary_handler_t	handler;

	create_test_collection_std_conditions(&dict, &handler);

	dict_cursor_t	*cursor;
	predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(5, int), IONIZE(78, int));

	err_t status = dictionary_find(&dict, &predicate, &cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status);
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;

	record.key		= malloc(dict.instance->record.key_size);
	record.value	= malloc(dict.instance->record.value_size);

	cursor_status_t c_status = cursor->next(cursor, &record);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active == c_status);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(5, int), dict.instance->record.key_size) >= 0);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(78, int), dict.instance->record.key_size) <= 0);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.value, (ion_value_t) (char *) { "DATA" }, dict.instance->record.value_size) == 0);

	while (cursor->next(cursor, &record) != cs_end_of_results) {
		PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(5, int), dict.instance->record.key_size) >= 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.key, IONIZE(78, int), dict.instance->record.key_size) <= 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, dict.instance->compare(record.value, (ion_value_t) (char *) { "DATA" }, dict.instance->record.value_size) == 0);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->status);

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

	/* Creation tests */
	planck_unit_add_to_suite(suite, test_collection_handler_binding);
	planck_unit_add_to_suite(suite, test_collection_creation);

	/* Cursor Equality tests */
	planck_unit_add_to_suite(suite, test_slhandler_cursor_equality);
	planck_unit_add_to_suite(suite, test_slhandler_cursor_equality_with_results);

	/* Cursor Range tests */
	planck_unit_add_to_suite(suite, test_slhandler_cursor_range);
	planck_unit_add_to_suite(suite, test_slhandler_cursor_range_with_results);

	return suite;
}

/**
@brief	  Runs all skiplist related tests and outputs the result.
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
