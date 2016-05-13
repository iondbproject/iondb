/**
@file 		test_flat_file_dictionary_handler.c

@author		Scott Ronald Fazackerley

 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "../../../planck_unit.h"
#include "../../../../dictionary/dictionary_types.h"
#include "./../../../../dictionary/dictionary.h"
#include "./../../../../dictionary/flat_file/flat_file.h"
#include "../../../../dictionary/flat_file/flat_file_dictionary_handler.h"

#define TEST_FILE	"file.bin"
/**
@brief		A helper function to build a test collection

@param 		map_handler
@param 		record
@param 		size
@param 		test_dictionary
*/
void
createFlatFileTestCollection(
    dictionary_handler_t	*map_handler,
    const record_info_t		*record,
    int 					size,
    dictionary_t			*test_dictionary,
    key_type_t				key_type
)
{
	ffdict_init(map_handler); //register handler for hashmap
	//register the appropriate handler for a given collection

	dictionary_create(map_handler, test_dictionary, 1, key_type,
	        record->key_size, record->value_size, size);
	//build test relation
	int i;
	ion_value_t str;
	str = (ion_value_t)malloc(record->value_size * 2);
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
				plank_unit_test_t
 */
void
test_flat_file_handler_function_registration(
	planck_unit_test_t	*tc
)
{
	dictionary_handler_t map_handler;			//create handler for hashmap

	ffdict_init(&map_handler);					//register handler for hashmap

	//check to ensure that function pointers are correctly registered
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.insert				== &ffdict_insert);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.create_dictionary	== &ffdict_create_dictionary);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.update				== &ffdict_update);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.remove				== &ffdict_delete);
	PLANCK_UNIT_ASSERT_TRUE(tc, map_handler.delete_dictionary	== &ffdict_delete_dictionary);

}

/**
@brief 		Tests for creatation and deletion of open address hash.

@param 		tc
				plank_unit_test_t
 */
void
test_flat_file_handler_create_destroy(
	planck_unit_test_t	*tc
)
{
	record_info_t record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size = 4;
	record.value_size = 10;

	dictionary_handler_t map_handler;			//create handler for hashmap

	ffdict_init(&map_handler);					//register handler for hashmap

	//collection handler for test collection
	dictionary_t test_dictionary;

	//register the appropriate handler for a given collection
	dictionary_create(&map_handler, &test_dictionary, 1, key_type_numeric_signed, record.key_size, record.value_size, 0);

	PLANCK_UNIT_ASSERT_TRUE(tc, (((ff_file_t *)test_dictionary.instance)->super.record.key_size) == record.key_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, (((ff_file_t *)test_dictionary.instance)->super.record.value_size) == record.value_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, (((ff_file_t *)test_dictionary.instance)->write_concern) == wc_insert_unique);

	//check to see if the file has been created and read the data back
	frewind(((ff_file_t *)test_dictionary.instance)->file_ptr);

	ff_file_t file;
	PLANCK_UNIT_ASSERT_TRUE(tc, 1						== fread(&(file.super),sizeof(file.super),1,((ff_file_t *)test_dictionary.instance)->file_ptr));
	PLANCK_UNIT_ASSERT_TRUE(tc, record.key_size  		== file.super.record.key_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, record.value_size		== file.super.record.value_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, key_type_numeric_signed
											== file.super.key_type);
	PLANCK_UNIT_ASSERT_TRUE(tc, dictionary_compare_signed_value
											== file.super.compare);
/** @FIXME - Test fails
	PLANCK_UNIT_ASSERT_TRUE(tc, 0						!= feof(((ff_file_t *)test_dictionary.instance)->file_ptr));
*/
	//delete the dictionary
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok
											== test_dictionary.handler->delete_dictionary(&test_dictionary) );
	PLANCK_UNIT_ASSERT_TRUE(tc, test_dictionary.instance
											== NULL);

	//and check the status of the file (not being there)
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL					== fopen("test.bin","rb"));
}

/**
@brief 		Tests a simple insert into dictionary and simple query

@details	Tests a simple insert into dictionary and simple query with the
			write_concern set to insert only
@param 		tc
				plank_unit_test_t
 */
void
test_flat_file_handler_simple_insert(
	planck_unit_test_t	*tc
)
{
	record_info_t record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size = 4;
	record.value_size = 10;

	int test_key = 1;
	char test_value[record.value_size * 2];

	dictionary_handler_t dict_handler;			//create handler for hashmap

	ffdict_init(&dict_handler);					//register handler for hashmap

	//collection handler for test collection
	dictionary_t test_dictionary;

	//register the appropriate handler for a given collection
	dictionary_create(&dict_handler, &test_dictionary, 1, key_type_numeric_signed, record.key_size, record.value_size, 0);

	sprintf((char*)test_value, "value : %i ", test_key);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == test_dictionary.handler->insert(&test_dictionary,(ion_key_t)&test_key,(ion_value_t)test_value));

	//reset cursor on file and
	fseek(((ff_file_t *)test_dictionary.instance)->file_ptr, ((ff_file_t *)test_dictionary.instance)->start_of_data, SEEK_SET);

	//ff_file_record_t file_record;
	f_file_record_t * file_record;  /* this needs to be dynamically allocated */

	int record_size = SIZEOF(STATUS) + test_dictionary.instance->record.key_size + test_dictionary.instance->record.value_size;

	file_record = (f_file_record_t *)malloc(sizeof(char) * record_size);

	//read the record_info back and check
	PLANCK_UNIT_ASSERT_TRUE(tc, 1					== fread(file_record, record_size, 1, ((ff_file_t *)test_dictionary.instance)->file_ptr));

	PLANCK_UNIT_ASSERT_TRUE(tc, IN_USE	 			== file_record->status);

	PLANCK_UNIT_ASSERT_TRUE(tc, 0 					== memcmp((char *)(file_record->data+test_dictionary.instance->record.key_size), test_value,test_dictionary.instance->record.value_size));

	PLANCK_UNIT_ASSERT_TRUE(tc, test_key			== *(int*)(file_record->data));

	test_key++;

	sprintf((char*)test_value, "value : %i ", test_key);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == test_dictionary.handler->insert(&test_dictionary,(ion_key_t)&test_key,(ion_value_t)test_value));

	// TODO Check why this flag is not being set
	// PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false	!= feof((((ff_file_t *)test_dictionary.instance)->file_ptr)));

	//reset cursor on file and
	fseek(((ff_file_t *)test_dictionary.instance)->file_ptr, ((ff_file_t *)test_dictionary.instance)->start_of_data, SEEK_SET);

	int i;
	for (i = 1;i<3;i++)
	{
		sprintf((char*)test_value, "value : %i ", i);
		//read the record_info back and check
		PLANCK_UNIT_ASSERT_TRUE(tc, 1					== fread(file_record, record_size, 1, ((ff_file_t *)test_dictionary.instance)->file_ptr));
		//check status
		PLANCK_UNIT_ASSERT_TRUE(tc, IN_USE	 			== file_record->status);
		//check value
		PLANCK_UNIT_ASSERT_TRUE(tc, 0 == memcmp((char *)(file_record->data+test_dictionary.instance->record.key_size), test_value,test_dictionary.instance->record.value_size));
		//check key
		PLANCK_UNIT_ASSERT_TRUE(tc, i					== *(int*)(file_record->data));
	}

	//delete the dictionary
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == test_dictionary.handler->delete_dictionary(&test_dictionary));
	PLANCK_UNIT_ASSERT_TRUE(tc, test_dictionary.instance == NULL);
	free(file_record);
}

void
test_flat_file_dictionary_cursor_equality(
	planck_unit_test_t	*tc
)
{
	int 			size;
	record_info_t 	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size 	= 4;
	record.value_size 	= 10;
	size 				= 10;

	dictionary_handler_t 	file_handler;			//create handler for hashmap
	dictionary_t			test_dictionary;		//collection handler for test collection

	createFlatFileTestCollection(&file_handler, &record, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(1));
	//test that the query runs on collection okay
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 				== dictionary_find(&test_dictionary, &predicate, &cursor));

	//check the status of the cursor as it should be initialized
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized	== cursor->status);

	//destroy the cursor
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL 					== cursor);

	//and destory the collection
	dictionary_delete_dictionary(&test_dictionary);
}

void
test_flat_file_dictionary_handler_query_with_results(
	planck_unit_test_t	*tc
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

	createFlatFileTestCollection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(1));

	//test that the query runs on collection okay
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 					== dictionary_find(&test_dictionary, &predicate, &cursor));

	//check the status of the cursor as it should be initialized
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized	== cursor->status);

	//user must allocate memory before calling next()
	ion_record_t			record;
	record.value 			= (ion_value_t)malloc(sizeof(ion_value_t)*record_info.value_size);
	record.key 				= (ion_key_t)malloc(sizeof(ion_value_t)*record_info.key_size);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active		== cursor->next(cursor, &record));

	//check that value is correct that has been returned
	ion_value_t				str;
	str 					= (ion_value_t)malloc(record_info.value_size * 2);
	sprintf((char*)str,"value : %i ", *(int *)predicate.statement.equality.equality_value);

	PLANCK_UNIT_ASSERT_TRUE(tc, IS_EQUAL				== memcmp(record.value, str, record_info.value_size));

	free(str);

	//and as there is only 1 result, the next call should return empty
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results		== cursor->next(cursor, &record));

	//and as there is only 1 result, the next call should return empty
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results		== cursor->next(cursor, &record));

	free(record.value);
	free(record.key);
	//destory cursor for cleanup
	cursor->destroy(&cursor);
	//and destory the collection
	dictionary_delete_dictionary(&test_dictionary);
}

void
test_flat_file_dictionary_handler_query_no_results(
	planck_unit_test_t	*tc
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

	createFlatFileTestCollection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(-1));

	//test that the query runs on collection okay
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 					== dictionary_find(&test_dictionary, &predicate, &cursor));

	//check the status of the cursor as it should be at the end of results as no values exist
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results		== cursor->status);

	//user must allocate memory before calling next()
	ion_record_t 			record;
	record.key 					= (ion_key_t)malloc(sizeof(ion_key_t)*record_info.value_size);
	record.value 					= (ion_value_t)malloc(sizeof(ion_value_t)*record_info.value_size);

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results		== cursor->next(cursor, &record));

	free(record.key);
	free(record.value);
	//destroy cursor for cleanup
	cursor->destroy(&cursor);
	//and destroy the collection
	dictionary_delete_dictionary(&test_dictionary);
}

void
test_flat_file_dictionary_predicate_equality(
	planck_unit_test_t	*tc
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

	createFlatFileTestCollection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);
	dict_cursor_t 			*cursor;				//create a new cursor pointer

	cursor = (dict_cursor_t *)malloc(sizeof(dict_cursor_t));
	cursor->destroy 		= ffdict_destroy_cursor;

	//create a new predicate statement
	predicate_t 			predicate;
	dictionary_build_predicate(&predicate, predicate_equality, IONIZE(1));

	cursor->dictionary 		= &test_dictionary;				//register test dictionary
	cursor->predicate 		= &predicate;					//register predicate

	memcpy(key_under_test,(ion_key_t)&(int){1},sizeof(int));
	
	//printf("key %i\n",*(int *)key_under_test);
	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true 	== ffdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){2},sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false 	== ffdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){-1},sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false 	== ffdict_test_predicate(cursor, key_under_test));

	free(cursor);
	free(key_under_test);

	//and destroy the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_flat_file_dictionary_predicate_range_signed(
	planck_unit_test_t	*tc
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

	createFlatFileTestCollection(&map_handler, &record_info, size, &test_dictionary,key_type_numeric_signed);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	cursor = (dict_cursor_t *)malloc(sizeof(dict_cursor_t));
	cursor->destroy 		= ffdict_destroy_cursor;

	//create a new predicate statement
	predicate_t 			predicate;
	dictionary_build_predicate(&predicate, predicate_range, IONIZE(-1), IONIZE(1));

	cursor->dictionary 		= &test_dictionary;				//register test dictionary
	cursor->predicate 		= &predicate;					//register predicate

	memcpy(key_under_test,(ion_key_t)&(int){0},sizeof(int));

	//printf("key %i\n",*(int *)key_under_test);

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true 	== ffdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){-1},sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true 	== ffdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){1},sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true 	== ffdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){2},sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false 	== ffdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){-2},sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false 	== ffdict_test_predicate(cursor, key_under_test));

	free(key_under_test);
	free(cursor);

	//and destroy the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}
void
test_flat_file_dictionary_predicate_range_unsigned(
	planck_unit_test_t	*tc
)
{
	ion_key_t key_under_test;
	key_under_test = (ion_key_t)malloc(sizeof(unsigned int));

	int size;
	record_info_t record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size = 4;
	record_info.value_size = 10;
	size = 10;

	dictionary_handler_t 	map_handler;					//create handler for hashmap
	dictionary_t 			test_dictionary;				//collection handler for test collection

	createFlatFileTestCollection(&map_handler, &record_info, size, &test_dictionary,key_type_numeric_unsigned);

	dict_cursor_t 			*cursor;						//create a new cursor pointer

	cursor = (dict_cursor_t *)malloc(sizeof(dict_cursor_t));
	cursor->destroy 		= ffdict_destroy_cursor;

	//create a new predicate statement
	predicate_t 			predicate;
	dictionary_build_predicate(&predicate, predicate_range, IONIZE(0), IONIZE(2));

	cursor->dictionary 		= &test_dictionary;				//register test dictionary
	cursor->predicate 		= &predicate;					//register predicate

	memcpy(key_under_test,(ion_key_t)&(unsigned int){0},sizeof(unsigned int));

	//printf("key %i\n",*(unsigned int *)key_under_test);

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true 	== ffdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){1},sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true 	== ffdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){2},sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true 	== ffdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){3},sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false 	== ffdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){4},sizeof(unsigned int));

	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_false 	== ffdict_test_predicate(cursor, key_under_test));

	free(key_under_test);
	free(cursor);
	
	//and destroy the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_flat_file_dictionary_cursor_range(
	planck_unit_test_t	*tc
)
{
	int 			size;
	record_info_t 	record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size 	= 4;
	record_info.value_size 	= 10;
	size 					= 10;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t			test_dictionary;		//collection handler for test collection

	createFlatFileTestCollection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	dictionary_build_predicate(&predicate, predicate_range, IONIZE(1), IONIZE(5));
	//test that the query runs on collection okay
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok 						== dictionary_find(&test_dictionary, &predicate, &cursor));

	//check the status of the cursor as it should be initialized
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized		== cursor->status);

	//user must allocate memory before calling next()
	ion_record_t 					record;
	record.key 						= (ion_key_t)malloc(record_info.key_size);
	record.value 					= (ion_value_t)malloc(record_info.value_size);

	int result_count = 0;
	status_t cursor_status;

	while( cs_cursor_active == (cursor_status = cursor->next(cursor, &record)))
	{

		PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_active		== cursor_status);

		//check that value is correct that has been returned
		ion_value_t	str;
		str = (ion_value_t)malloc(record_info.value_size * 2);
		sprintf((char*)str,"value : %i ", (*(int *)predicate.statement.range.lower_bound) + result_count);

		PLANCK_UNIT_ASSERT_TRUE(tc, IS_EQUAL				== memcmp(record.value, str, record_info.value_size));

		result_count++;
		free(str);
	}
	PLANCK_UNIT_ASSERT_TRUE(tc, 5							== result_count);

	//and as there is only 1 result, the next call should return empty
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results			== cursor->next(cursor, &record));

	//free up record
	free(record.key);
	free(record.value);

	//destroy the cursor
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL 						== cursor);

	//and destory the collection
	dictionary_delete_dictionary(&test_dictionary);
}

planck_unit_suite_t*
flat_file_handler_getsuite()
{
	planck_unit_suite_t *suite = planck_unit_new_suite();
	planck_unit_add_to_suite(suite, test_flat_file_handler_function_registration);
	planck_unit_add_to_suite(suite, test_flat_file_handler_create_destroy);
	planck_unit_add_to_suite(suite, test_flat_file_handler_simple_insert);
	/** @tdodo need simple query */
	planck_unit_add_to_suite(suite, test_flat_file_dictionary_predicate_equality);
	planck_unit_add_to_suite(suite, test_flat_file_dictionary_predicate_range_signed);
	planck_unit_add_to_suite(suite, test_flat_file_dictionary_predicate_range_unsigned);
	planck_unit_add_to_suite(suite, test_flat_file_dictionary_cursor_equality);
	planck_unit_add_to_suite(suite, test_flat_file_dictionary_handler_query_with_results);
	planck_unit_add_to_suite(suite, test_flat_file_dictionary_handler_query_no_results);
	planck_unit_add_to_suite(suite, test_flat_file_dictionary_cursor_range);

	return suite;
}

void
runalltests_flat_file_handler()
{
	planck_unit_suite_t		*suite	= flat_file_handler_getsuite();

	planck_unit_run_suite(suite);
}


