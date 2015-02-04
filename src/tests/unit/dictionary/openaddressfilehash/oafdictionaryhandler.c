/**
@file 		oadictionaryhandler.c

@author		Scott Ronald Fazackerley

 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/openaddressfilehash/oafhash.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"
#include "./../../../../dictionary/openaddressfilehash/oafdictionaryhandler.h"

#define MAX_HASH_TEST 100

/**
@brief		A helper function to build a test collection

@param 		map_handler
@param 		record
@param 		size
@param 		test_dictionary
 */
void
createFileTestCollection(
    dictionary_handler_t	*map_handler,
    const record_info_t			*record,
    int 					size,
    dictionary_t			*test_dictionary,
    key_type_t				key_type
)
{
	oafdict_init(map_handler); //register handler for hashmap
	//register the appropriate handler for a given collection

	dictionary_create(map_handler, test_dictionary, key_type,
	        record->key_size, record->value_size, size);
	//build test relation
	int i;
	ion_value_t str;
	str = (ion_value_t)malloc(record->value_size);
	for (i = 0; i < size; i++)
	{
		sprintf((char*)str, "value : %i ", i);
		test_dictionary->handler->insert(test_dictionary, (ion_key_t)&i, str);
	}
	free(str);
}
/**
@brief		Validates the correct registration of function pointers
			for open address dictionary structure.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_function_registration(
	CuTest		*tc
)
{
	dictionary_handler_t map_handler;			//create handler for hashmap

	oafdict_init(&map_handler);					//register handler for hashmap

	//check to ensure that function pointers are correctly registered
	CuAssertTrue(tc, map_handler.insert				== &oafdict_insert);
	CuAssertTrue(tc, map_handler.create_dictionary	== &oafdict_create_dictionary);
	CuAssertTrue(tc, map_handler.update				== &oafdict_update);
	CuAssertTrue(tc, map_handler.remove				== &oafdict_delete);
	CuAssertTrue(tc, map_handler.delete_dictionary	== &oafdict_delete_dictionary);

}

/**
@brief 		Tests for creatation and deletion of open address hash.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_create_destroy(
	CuTest		*tc
)
{

	int size;
	record_info_t record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size = 4;
	record.value_size = 10;
	size = 10;

	dictionary_handler_t map_handler;			//create handler for hashmap

	oafdict_init(&map_handler);					//register handler for hashmap

	//collection handler for test collection
	dictionary_t test_dictionary;

	//register the appropriate handler for a given collection
	dictionary_create(&map_handler, &test_dictionary, key_type_numeric_signed, record.key_size, record.value_size, size);

	CuAssertTrue(tc, (((file_hashmap_t *)test_dictionary.instance)->super.record.key_size) ==  record.key_size);
	CuAssertTrue(tc, (((file_hashmap_t *)test_dictionary.instance)->super.record.value_size) == record.value_size);
	CuAssertTrue(tc, (((file_hashmap_t *)test_dictionary.instance)->map_size) == size);
	CuAssertTrue(tc, (((file_hashmap_t *)test_dictionary.instance)->compute_hash) == &oafh_compute_simple_hash);
	CuAssertTrue(tc, (((file_hashmap_t *)test_dictionary.instance)->write_concern) == wc_insert_unique);
	CuAssertTrue(tc, test_dictionary.handler->delete_dictionary(&test_dictionary) == err_ok);
	CuAssertTrue(tc, test_dictionary.instance == NULL);

}

/**
@brief 		Tests a simple insert into dictionary and simple query

@details	Tests a simple insert into dictionary and simple query with the
			write_concern set to insert only
@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_simple_insert_and_query(
	CuTest		*tc
)
{

}

/**
@brief 		Tests a simple delete from dictionary

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_simple_delete(
	CuTest		*tc
)
{

}

/**
@brief		Tests that duplicate values are not allowed with write_concern
			is set for wc_insert_unique.

@details	Tests that duplicate values are not allowed with write_concern
			is set for wc_insert_unique.   Will generate error.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_duplicate_insert_1(
	CuTest		*tc
)
{

}

/**
@brief		Tests that value updates are allowed with write_concern
			is set for wc_update.

@details	Tests that duplicate values are not allowed with write_concern
			is set for wc_update but will update the value.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_duplicate_insert_2(
	CuTest		*tc
)
{
}

/**
@brief		Tests that values can be updated.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_update_1(
	CuTest		*tc
)
{
}

/**
@brief		Tests that values will be inserted on update if they do not exist
 	 	 	in dictionary already.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_update_2(
	CuTest		*tc
)
{
}

/**
@brief		Tests that values can be deleted from dictionary with single value.

@param 		tc
				CuTeest
 */
void
test_open_address_file_hashmap_handler_delete_1(
	CuTest		*tc
)
{
}

/**
@brief		Tests that values can be deleted from dictionary with multiple
			values.

@details	Tests that values can be deleted from dictionary with multiple
			values.  Dictionary will have multiple values and only a single
			value will be deleted.  Checks to ensure that other values are
			undisturbed.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_delete_2(
	CuTest		*tc
)
{
}

/**
@brief		Tests that values can be inserted until capacity has been reached.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_handler_capacity(
	CuTest		*tc
)
{
}



void
test_open_address_file_dictionary_cursor_equality(
	CuTest		*tc
)
{
	int 		size;
	record_info_t 	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size 	= 4;
	record.value_size 	= 10;
	size 				= 10;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t			test_dictionary;		//collection handler for test collection

	createFileTestCollection(&map_handler, &record, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t 			*cursor;			//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type = predicate_equality;
	//need to prepare predicate correctly
	predicate.statement.equality.equality_value = (ion_key_t)malloc(sizeof(int));

	/** @Todo This needs to be improved */
	memcpy(predicate.statement.equality.equality_value,(ion_key_t)&(int){1},sizeof(int));

	//test that the query runs on collection okay
	CuAssertTrue(tc, err_ok 				== test_dictionary.handler->find(&test_dictionary, &predicate, &cursor));

	//check the status of the cursor as it should be initialized
	CuAssertTrue(tc, cs_cursor_initialized	== cursor->status);

	//free up the correct predicate
	free(predicate.statement.equality.equality_value);

	//destroy the cursor
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_open_address_file_dictionary_handler_query_with_results(
	CuTest		*tc
)
{
	int size;
	record_info_t record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size = 4;
	record_info.value_size = 10;
	size = 10;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	createFileTestCollection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type 			= predicate_equality;

	//need to prepare predicate correctly
	predicate.statement.equality.equality_value = (ion_key_t)malloc(sizeof(int));
	/** @Todo This needs to be improved */
	memcpy(predicate.statement.equality.equality_value,(ion_key_t)&(int){1},sizeof(int));

	//test that the query runs on collection okay
	CuAssertTrue(tc, err_ok 				== test_dictionary.handler->find(&test_dictionary, &predicate, &cursor));

	//check the status of the cursor as it should be initialized
	CuAssertTrue(tc, cs_cursor_initialized	== cursor->status);

	//user must allocate memory before calling next()
	ion_record_t			record;
	record.key				= (ion_key_t)malloc(record_info.key_size);
	record.value 			= (ion_value_t)malloc(record_info.value_size);


	CuAssertTrue(tc, cs_cursor_active		== cursor->next(cursor, &record));

	//check that value is correct that has been returned
	ion_value_t				str;
	str 					= (ion_value_t)malloc(record_info.value_size);
	sprintf((char*)str,"value : %i ", *(int *)predicate.statement.equality.equality_value);

	CuAssertTrue(tc, IS_EQUAL				== memcmp(record.value, str, record_info.value_size));

	free(str);

	//and as there is only 1 result, the next call should return empty
	CuAssertTrue(tc, cs_end_of_results		== cursor->next(cursor, &record));

	//and as there is only 1 result, the next call should return empty
	CuAssertTrue(tc, cs_end_of_results		== cursor->next(cursor, &record));

	//free up the correct predicate
	free(predicate.statement.equality.equality_value);
	free(record.key);
	free(record.value);

	//destory cursor for cleanup
	cursor->destroy(&cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_open_address_file_dictionary_handler_query_no_results(
	CuTest		*tc
)
{
	int size;
	record_info_t record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size = 4;
	record_info.value_size = 10;
	size = 10;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	createFileTestCollection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type 			= predicate_equality;

	//need to prepare predicate correctly
	predicate.statement.equality.equality_value = (ion_key_t)malloc(sizeof(int));
	/** @Todo This needs to be improved */
	memcpy(predicate.statement.equality.equality_value,(ion_key_t)&(int){-1},sizeof(int));

	//test that the query runs on collection okay
	CuAssertTrue(tc, err_ok 				== test_dictionary.handler->find(&test_dictionary, &predicate, &cursor));

	//check the status of the cursor as it should be at the end of results as no values exist
	CuAssertTrue(tc, cs_end_of_results	== cursor->status);

	//user must allocate memory before calling next()
	ion_record_t 			record;

	record.key 				= (ion_key_t)malloc(record_info.key_size);
	record.value 			= (ion_value_t)malloc(record_info.value_size);

	CuAssertTrue(tc, cs_end_of_results		== cursor->next(cursor, &record));

	//free up the correct predicate
	free(predicate.statement.equality.equality_value);

	free(record.key);
	free(record.value);

	//destroy cursor for cleanup
	cursor->destroy(&cursor);

	//and destroy the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_open_address_file_dictionary_predicate_equality(
	CuTest		*tc
)
{
	ion_key_t key_under_test;
	key_under_test = (ion_key_t)malloc(sizeof(int));

	int size;
	record_info_t record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size = 4;
	record.value_size = 10;
	size = 10;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	createFileTestCollection(&map_handler, &record, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	cursor = (dict_cursor_t *)malloc(sizeof(dict_cursor_t));
	cursor->destroy 		= oafdict_destroy_cursor;

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type 			= predicate_equality;

	//need to prepare predicate correctly
	predicate.statement.equality.equality_value = (ion_key_t)malloc(sizeof(int));
	memcpy(predicate.statement.equality.equality_value,(ion_key_t)&(int){1},sizeof(int));

	cursor->dictionary 		= &test_dictionary;				//register test dictionary
	cursor->predicate 		= &predicate;					//register predicate
	cursor->type			= cursor_equality;

	memcpy(key_under_test,(ion_key_t)&(int){1},sizeof(int));

	//printf("key %i\n",*(int *)key_under_test);

	CuAssertTrue(tc, boolean_true 	== oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){2},sizeof(int));

	CuAssertTrue(tc, boolean_false 	== oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){-1},sizeof(int));

	CuAssertTrue(tc, boolean_false 	== oafdict_test_predicate(cursor, key_under_test));

	free(key_under_test);

	//free up the correct predicate
	free(predicate.statement.equality.equality_value);
	//destroy cursor for cleanup
	cursor->destroy(&cursor);
	//and destroy the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_open_address_file_dictionary_predicate_range_signed(
	CuTest		*tc
)
{
	ion_key_t key_under_test;
	key_under_test = (ion_key_t)malloc(sizeof(int));

	int size;
	record_info_t record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size = 4;
	record_info.value_size = 10;
	size = 10;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	createFileTestCollection(&map_handler, &record_info, size, &test_dictionary,key_type_numeric_signed);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	cursor = (dict_cursor_t *)malloc(sizeof(dict_cursor_t));
	cursor->destroy 		= oafdict_destroy_cursor;

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type 			= predicate_range;

	//need to prepare predicate correctly
	predicate.statement.range.geq_value = (ion_key_t)malloc(sizeof(int));
	predicate.statement.range.leq_value = (ion_key_t)malloc(sizeof(int));

	memcpy(predicate.statement.range.geq_value,(ion_key_t)&(int){-1},sizeof(int));
	memcpy(predicate.statement.range.leq_value,(ion_key_t)&(int){1},sizeof(int));

	cursor->dictionary 		= &test_dictionary;				//register test dictionary
	cursor->predicate 		= &predicate;					//register predicate
	cursor->type			= cursor_range;

	memcpy(key_under_test, (ion_key_t)&(int){0}, sizeof(int));

	//DUMP(*(int *)key_under_test,"%i");

	CuAssertTrue(tc, boolean_true 	== oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){-1},sizeof(int));

	CuAssertTrue(tc, boolean_true 	== oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){1},sizeof(int));

	CuAssertTrue(tc, boolean_true 	== oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){2},sizeof(int));

	CuAssertTrue(tc, boolean_false 	== oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){-2},sizeof(int));

	CuAssertTrue(tc, boolean_false 	== oafdict_test_predicate(cursor, key_under_test));

	free(key_under_test);

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);
	//destroy cursor for cleanup
	cursor->destroy(&cursor);
	//and destroy the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}
void
test_open_address_file_dictionary_predicate_range_unsigned(
	CuTest		*tc
)
{
	ion_key_t key_under_test;
	key_under_test = (ion_key_t)malloc(sizeof(unsigned int));

	int size;
	record_info_t record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size = 4;
	record.value_size = 10;
	size = 10;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	createFileTestCollection(&map_handler, &record, size, &test_dictionary,key_type_numeric_unsigned);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	cursor = (dict_cursor_t *)malloc(sizeof(dict_cursor_t));
	cursor->destroy 		= oafdict_destroy_cursor;

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type 			= predicate_range;

	//need to prepare predicate correctly
	predicate.statement.range.geq_value = (ion_key_t)malloc(sizeof(unsigned int));
	predicate.statement.range.leq_value = (ion_key_t)malloc(sizeof(unsigned int));

	memcpy(predicate.statement.range.geq_value,(ion_key_t)&(unsigned int){0},sizeof(unsigned int));
	memcpy(predicate.statement.range.leq_value,(ion_key_t)&(unsigned int){2},sizeof(unsigned int));

	cursor->dictionary 		= &test_dictionary;				//register test dictionary
	cursor->predicate 		= &predicate;					//register predicate
	cursor->type			= cursor_range;

	memcpy(key_under_test,(ion_key_t)&(unsigned int){0},sizeof(unsigned int));

	//printf("key %i\n",*(unsigned int *)key_under_test);

	CuAssertTrue(tc, boolean_true 	== oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){1},sizeof(unsigned int));

	CuAssertTrue(tc, boolean_true 	== oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){2},sizeof(unsigned int));

	CuAssertTrue(tc, boolean_true 	== oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){3},sizeof(unsigned int));

	CuAssertTrue(tc, boolean_false 	== oafdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){4},sizeof(unsigned int));

	CuAssertTrue(tc, boolean_false 	== oafdict_test_predicate(cursor, key_under_test));

	free(key_under_test);

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);
	//destroy cursor for cleanup
	cursor->destroy(&cursor);
	//and destroy the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);

}

void
test_open_address_file_dictionary_cursor_range(
	CuTest		*tc
)
{
	int 		size;
	record_info_t 	record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size 	= 4;
	record_info.value_size 	= 10;
	size 					= 10;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t			test_dictionary;		//collection handler for test collection

	createFileTestCollection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t 			*cursor;			//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type = predicate_range;
	//need to prepare predicate correctly
	predicate.statement.range.geq_value = (ion_key_t)malloc(sizeof(int));

	memcpy(predicate.statement.range.geq_value,(ion_key_t)&(int){1},sizeof(int));

	predicate.statement.range.leq_value = (ion_key_t)malloc(sizeof(int));

	memcpy(predicate.statement.range.leq_value,(ion_key_t)&(int){5},sizeof(int));

	//test that the query runs on collection okay
	CuAssertTrue(tc, err_ok 				== test_dictionary.handler->find(&test_dictionary, &predicate, &cursor));

	//check the status of the cursor as it should be initialized
	CuAssertTrue(tc, cs_cursor_initialized	== cursor->status);

	//user must allocate memory before calling next()
	ion_record_t 				record;

	record.key 					= (ion_key_t)malloc(record_info.key_size);
	record.value 				= (ion_value_t)malloc(record_info.value_size);

	int result_count 			= 0;
	status_t 					cursor_status;

	while( cs_cursor_active == (cursor_status = cursor->next(cursor, &record)))
	{
		CuAssertTrue(tc, cs_cursor_active		== cursor_status);

		//check that value is correct that has been returned
		ion_value_t	str;
		str = (ion_value_t)malloc(record_info.value_size);
		sprintf((char*)str,"value : %i ", (*(int *)predicate.statement.range.geq_value) + result_count);

		CuAssertTrue(tc, IS_EQUAL				== memcmp(record.value, str, record_info.value_size));
		result_count++;
		free(str);
	}
	CuAssertTrue(tc, 5						== result_count);

	//and as there is only 1 result, the next call should return empty
	CuAssertTrue(tc, cs_end_of_results		== cursor->next(cursor, &record));

	free(record.key);
	free(record.value);

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);

	//destroy the cursor
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

CuSuite*
open_address_file_hashmap_handler_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_handler_function_registration);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_handler_create_destroy);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_handler_simple_insert_and_query);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_handler_simple_delete);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_handler_duplicate_insert_1);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_handler_duplicate_insert_2);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_handler_update_1);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_handler_update_2);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_handler_delete_1);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_handler_delete_2);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_handler_capacity);
	SUITE_ADD_TEST(suite, test_open_address_file_dictionary_predicate_equality);
	SUITE_ADD_TEST(suite, test_open_address_file_dictionary_predicate_range_signed);
	SUITE_ADD_TEST(suite, test_open_address_file_dictionary_predicate_range_unsigned);
	SUITE_ADD_TEST(suite, test_open_address_file_dictionary_cursor_equality);
	SUITE_ADD_TEST(suite, test_open_address_file_dictionary_handler_query_with_results);
	SUITE_ADD_TEST(suite, test_open_address_file_dictionary_handler_query_no_results);
	SUITE_ADD_TEST(suite, test_open_address_file_dictionary_cursor_range);

	return suite;
}

void
runalltests_open_address_file_hash_handler()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= open_address_file_hashmap_handler_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}


