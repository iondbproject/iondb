/**
@file

@author		Scott Ronald Fazackerley

 */

#if defined(__cplusplus)
extern "C" {
#endif

#include "test_open_address_file_hash_dictionary_handler.h"

#define MAX_HASH_TEST 100

/**
@brief		A helper function to build a test collection

@param	  map_handler
@param	  record
@param	  size
@param	  test_dictionary
 */
void
createFileTestCollection(
	dictionary_handler_t	*map_handler,
	const record_info_t		*record,
	int						size,
	dictionary_t			*test_dictionary,
	key_type_t				key_type
) {
	oafdict_init(map_handler);	/* register handler for hashmap */
	/* register the appropriate handler for a given collection */

	dictionary_create(map_handler, test_dictionary, 1, key_type, record->key_size, record->value_size, size);

	/* build test relation */
	int			i;
	ion_value_t str;

	str = (ion_value_t) malloc(record->value_size);

	for (i = 0; i < size; i++) {
		sprintf((char *) str, "value : %i", i);
		test_dictionary->handler->insert(test_dictionary, (ion_key_t) &i, str);
	}

	free(str);
}

/**
@brief		Validates the correct registration of function pointers
			for open address dictionary structure.

@param	  tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_function_registration(
	planck_unit_test_t *tc
) {
	dictionary_handler_t map_handler;	/* create handler for hashmap */

	oafdict_init(&map_handler);	/* register handler for hashmap */

	/* check to ensure that function pointers are correctly registered */
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.insert == &oafdict_insert);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.create_dictionary == &oafdict_create_dictionary);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.update == &oafdict_update);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.remove == &oafdict_delete);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.delete_dictionary == &oafdict_delete_dictionary);
}

/**
@brief	  Tests for creatation and deletion of open address hash.

@param	  tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_create_destroy(
	planck_unit_test_t *tc
) {
	int				size;
	record_info_t	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size		= sizeof(int);
	record.value_size	= 10;
	size				= 10;

	dictionary_handler_t map_handler;	/* create handler for hashmap */

	oafdict_init(&map_handler);	/* register handler for hashmap */

	/* collection handler for test collection */
	dictionary_t test_dictionary;

	/* register the appropriate handler for a given collection */
	dictionary_create(&map_handler, &test_dictionary, 1, key_type_numeric_signed, record.key_size, record.value_size, size);

	PLANCK_UNIT_ASSERT_TRUE(tc, (((file_hashmap_t *) test_dictionary.instance)->super.record.key_size) == record.key_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, (((file_hashmap_t *) test_dictionary.instance)->super.record.value_size) == record.value_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, (((file_hashmap_t *) test_dictionary.instance)->map_size) == size);
	PLANCK_UNIT_ASSERT_TRUE(tc, (((file_hashmap_t *) test_dictionary.instance)->compute_hash) == &oafh_compute_simple_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, (((file_hashmap_t *) test_dictionary.instance)->write_concern) == wc_insert_unique);
	PLANCK_UNIT_ASSERT_TRUE(tc, test_dictionary.handler->delete_dictionary(&test_dictionary) == err_ok);
	PLANCK_UNIT_ASSERT_TRUE(tc, test_dictionary.instance == NULL);
}

void
test_open_address_file_dictionary_cursor_equality(
	planck_unit_test_t *tc
) {
	int				size;
	record_info_t	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size		= sizeof(int);
	record.value_size	= 10;
	size				= 10;

	dictionary_handler_t	map_handler;			/* create handler for hashmap */
	dictionary_t			test_dictionary;		/* collection handler for test collection */

	createFileTestCollection(&map_handler, &record, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t *cursor;	/* create a new cursor pointer */

	/* create a new predicate statement */
	predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(1, int));

	/* test that the query runs on collection okay */
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dictionary_find(&test_dictionary, &predicate, &cursor));

	/* check the status of the cursor as it should be initialized */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	/* destroy the cursor */
	cursor->destroy(&cursor);

	/* and check that cursor has been destroyed correctly */
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);

	/* and destory the collection */
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_open_address_file_dictionary_handler_query_with_results(
	planck_unit_test_t *tc
) {
	int				size;
	record_info_t	record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size	= sizeof(int);
	record_info.value_size	= 10;
	size					= 10;

	dictionary_handler_t	map_handler;			/* create handler for hashmap */
	dictionary_t			test_dictionary;		/* collection handler for test collection */

	createFileTestCollection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t *cursor;	/* create a new cursor pointer */

	/* create a new predicate statement */
	predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(1, int));

	/* test that the query runs on collection okay */
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dictionary_find(&test_dictionary, &predicate, &cursor));

	/* check the status of the cursor as it should be initialized */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	/* user must allocate memory before calling next() */
	ion_record_t record;

	record.key		= (ion_key_t) malloc(record_info.key_size);
	record.value	= (ion_value_t) malloc(record_info.value_size);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active == cursor->next(cursor, &record));

	/* check that value is correct that has been returned */
	ion_value_t str;

	str = (ion_value_t) malloc(record_info.value_size);
	sprintf((char *) str, "value : %i", *(int *) predicate.statement.equality.equality_value);

	PLANCK_UNIT_ASSERT_TRUE(tc, IS_EQUAL == memcmp(record.value, str, record_info.value_size));

	free(str);

	/* and as there is only 1 result, the next call should return empty */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->next(cursor, &record));

	/* and as there is only 1 result, the next call should return empty */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->next(cursor, &record));

	free(record.key);
	free(record.value);

	/* destory cursor for cleanup */
	cursor->destroy(&cursor);

	/* and destory the collection */
	dictionary_delete_dictionary(&test_dictionary);
}

void
test_open_address_file_dictionary_handler_query_no_results(
	planck_unit_test_t *tc
) {
	int				size;
	record_info_t	record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size	= sizeof(int);
	record_info.value_size	= 10;
	size					= 10;

	dictionary_handler_t	map_handler;			/* create handler for hashmap */
	dictionary_t			test_dictionary;		/* collection handler for test collection */

	createFileTestCollection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t *cursor;	/* create a new cursor pointer */

	/* create a new predicate statement */
	predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(-1, int));

	/* test that the query runs on collection okay */
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dictionary_find(&test_dictionary, &predicate, &cursor));

	/* check the status of the cursor as it should be at the end of results as no values exist */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->status);

	/* user must allocate memory before calling next() */
	ion_record_t record;

	record.key		= (ion_key_t) malloc(record_info.key_size);
	record.value	= (ion_value_t) malloc(record_info.value_size);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->next(cursor, &record));

	free(record.key);
	free(record.value);

	/* destroy cursor for cleanup */
	cursor->destroy(&cursor);

	/* and destroy the collection */
	dictionary_delete_dictionary(&test_dictionary);
}

void
test_open_address_file_dictionary_predicate_equality(
	planck_unit_test_t *tc
) {
	ion_key_t key_under_test;

	key_under_test = (ion_key_t) malloc(sizeof(int));

	int				size;
	record_info_t	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size		= sizeof(int);
	record.value_size	= 10;
	size				= 10;

	dictionary_handler_t	map_handler;			/* create handler for hashmap */
	dictionary_t			test_dictionary;		/* collection handler for test collection */

	createFileTestCollection(&map_handler, &record, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t *cursor;	/* create a new cursor pointer */

	cursor			= (dict_cursor_t *) malloc(sizeof(dict_cursor_t));
	cursor->destroy = oafdict_destroy_cursor;

	/* create a new predicate statement */
	predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(1, int));

	cursor->dictionary	= &test_dictionary;					/* register test dictionary */
	cursor->predicate	= &predicate;						/* register predicate */

	memcpy(key_under_test, (ion_key_t) &(int) { 1 }, sizeof(int));

	/* printf("key %i\n",*(int *)key_under_test); */

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test, (ion_key_t) &(int) { 2 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test, (ion_key_t) &(int) { -1 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == oafdict_test_predicate(cursor, key_under_test));

	free(key_under_test);
	free(cursor);

	/* destroy cursor for cleanup TODO TODO memory leak cannot free here!! */
	/* cursor->destroy(&cursor); */
	/* and destroy the collection */
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_open_address_file_dictionary_predicate_range_signed(
	planck_unit_test_t *tc
) {
	ion_key_t key_under_test;

	key_under_test = (ion_key_t) malloc(sizeof(int));

	int				size;
	record_info_t	record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size	= sizeof(int);
	record_info.value_size	= 10;
	size					= 10;

	dictionary_handler_t	map_handler;			/* create handler for hashmap */
	dictionary_t			test_dictionary;		/* collection handler for test collection */

	createFileTestCollection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t *cursor;	/* create a new cursor pointer */

	cursor			= (dict_cursor_t *) malloc(sizeof(dict_cursor_t));
	cursor->destroy = oafdict_destroy_cursor;

	/* create a new predicate statement */
	predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(-1, int), IONIZE(1, int));

	cursor->dictionary	= &test_dictionary;					/* register test dictionary */
	cursor->predicate	= &predicate;						/* register predicate */

	memcpy(key_under_test, (ion_key_t) &(int) { 0 }, sizeof(int));

	/* DUMP(*(int *)key_under_test,"%i"); */

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test, (ion_key_t) &(int) { -1 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test, (ion_key_t) &(int) { 1 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test, (ion_key_t) &(int) { 2 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test, (ion_key_t) &(int) { -2 }, sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == oafdict_test_predicate(cursor, key_under_test));

	free(key_under_test);

	free(cursor);

	/* destroy cursor for cleanup TODO TODO memory leak CANNOT free here!! */
	/* cursor->destroy(&cursor); */
	/* and destroy the collection */
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_open_address_file_dictionary_predicate_range_unsigned(
	planck_unit_test_t *tc
) {
	ion_key_t key_under_test;

	key_under_test = (ion_key_t) malloc(sizeof(unsigned int));

	int				size;
	record_info_t	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size		= sizeof(int);
	record.value_size	= 10;
	size				= 10;

	dictionary_handler_t	map_handler;			/* create handler for hashmap */
	dictionary_t			test_dictionary;		/* collection handler for test collection */

	createFileTestCollection(&map_handler, &record, size, &test_dictionary, key_type_numeric_unsigned);

	dict_cursor_t *cursor;	/* create a new cursor pointer */

	cursor			= (dict_cursor_t *) malloc(sizeof(dict_cursor_t));
	cursor->destroy = oafdict_destroy_cursor;

	/* create a new predicate statement */
	predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(0, int), IONIZE(2, int));

	cursor->dictionary	= &test_dictionary;					/* register test dictionary */
	cursor->predicate	= &predicate;						/* register predicate */

	memcpy(key_under_test, (ion_key_t) &(unsigned int) { 0 }, sizeof(unsigned int));

	/* printf("key %i\n",*(unsigned int *)key_under_test); */

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test, (ion_key_t) &(unsigned int) { 1 }, sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test, (ion_key_t) &(unsigned int) { 2 }, sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true == oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test, (ion_key_t) &(unsigned int) { 3 }, sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test, (ion_key_t) &(unsigned int) { 4 }, sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false == oafdict_test_predicate(cursor, key_under_test));

	free(key_under_test);

	free(cursor);

	/* destroy cursor for cleanup TODO TODO Memory leak here CANNOT free!!! */
	/* cursor->destroy(&cursor); */
	/* and destroy the collection */
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_open_address_file_dictionary_cursor_range(
	planck_unit_test_t *tc
) {
	int				size;
	record_info_t	record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size	= sizeof(int);
	record_info.value_size	= 10;
	size					= 10;

	dictionary_handler_t	map_handler;			/* create handler for hashmap */
	dictionary_t			test_dictionary;		/* collection handler for test collection */

	createFileTestCollection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t *cursor;	/* create a new cursor pointer */

	/* create a new predicate statement */
	predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_range, IONIZE(1, int), IONIZE(5, int));

	/* test that the query runs on collection okay */
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dictionary_find(&test_dictionary, &predicate, &cursor));

	/* check the status of the cursor as it should be initialized */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	/* user must allocate memory before calling next() */
	ion_record_t record;

	record.key		= (ion_key_t) malloc(record_info.key_size);
	record.value	= (ion_value_t) malloc(record_info.value_size);

	int				result_count = 0;
	cursor_status_t cursor_status;

	while (cs_cursor_active == (cursor_status = cursor->next(cursor, &record))) {
		PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active == cursor_status);

		/* check that value is correct that has been returned */
		ion_value_t str;

		str = (ion_value_t) malloc(record_info.value_size);
		sprintf((char *) str, "value : %i", (*(int *) predicate.statement.range.lower_bound) + result_count);

		PLANCK_UNIT_ASSERT_TRUE(tc, IS_EQUAL == memcmp(record.value, str, record_info.value_size));
		result_count++;
		free(str);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, 5 == result_count);

	/* and as there is only 1 result, the next call should return empty */
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->next(cursor, &record));

	free(record.key);
	free(record.value);

	/* destroy the cursor */
	cursor->destroy(&cursor);

	/* and check that cursor has been destroyed correctly */
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);

	/* and destory the collection */
	dictionary_delete_dictionary(&test_dictionary);
}

planck_unit_suite_t *
open_address_file_hashmap_handler_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	planck_unit_add_to_suite(suite, test_open_address_file_hashmap_handler_function_registration);
	planck_unit_add_to_suite(suite, test_open_address_file_hashmap_handler_create_destroy);
	planck_unit_add_to_suite(suite, test_open_address_file_dictionary_predicate_equality);
	planck_unit_add_to_suite(suite, test_open_address_file_dictionary_predicate_range_signed);
	planck_unit_add_to_suite(suite, test_open_address_file_dictionary_predicate_range_unsigned);
	planck_unit_add_to_suite(suite, test_open_address_file_dictionary_cursor_equality);
	planck_unit_add_to_suite(suite, test_open_address_file_dictionary_handler_query_with_results);
	planck_unit_add_to_suite(suite, test_open_address_file_dictionary_handler_query_no_results);
	planck_unit_add_to_suite(suite, test_open_address_file_dictionary_cursor_range);

	return suite;
}

void
runalltests_open_address_file_hash_handler(
) {
	/* CuString	*output	= CuStringNew(); */
	planck_unit_suite_t *suite = open_address_file_hashmap_handler_getsuite();

	planck_unit_run_suite(suite);
	/* CuSuiteSummary(suite, output); */
	/* CuSuiteDetails(suite, output); */
	/* printf("%s\n", output->buffer); */

	planck_unit_destroy_suite(suite);
	/* CuSuiteDelete(suite); */
	/* CuStringDelete(output); */
}
