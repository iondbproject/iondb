/**
@file 		ffdictionaryhandler.c

@author		Scott Ronald Fazackerley

 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/linearhash/linearhash.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"
#include "./../../../../dictionary/linearhash/lhdictionaryhandler.h"

#define TEST_FILE	"lh.bin"

/**
@brief		A helper function to build a test collection

@param 		map_handler
@param 		record
@param 		size
@param 		test_dictionary
*/
void
create_linear_hash_test_collection(
    dictionary_handler_t	*map_handler,
    const record_info_t		*record,
    int 					size,
    dictionary_t			*test_dictionary,
    key_type_t				key_type,
    int						num_keys
)
{

	ion_dictionary_id_t id = 0;

	lhdict_init(map_handler); //register handler for hashmap
	//register the appropriate handler for a given collection

	dictionary_create(map_handler, test_dictionary, key_type, id,
	        record->key_size, record->value_size, size);
	//build test relation
	int i;
	ion_value_t str;
	str = (ion_value_t)malloc(sizeof(ion_value_t) * record->value_size);
	for (i = 0; i < num_keys; i++)
	{
		sprintf((char*)str, "value : %i ", i);
		test_dictionary->handler->insert(test_dictionary, (ion_key_t)&i, str);
#if DEBUG
		DUMP(*(ion_key_t)&i,"%i");
#endif
	}
	free(str);
}

/**
@brief		Validates the correct registration of function pointers
			for disk based linear hash.

@param 		tc
				CuTest
 */
void
test_linear_hash_handler_function_registration(
	CuTest		*tc
)
{
	dictionary_handler_t map_handler;			//create handler for hashmap

	lhdict_init(&map_handler);					//register handler for hashmap

	//check to ensure that function pointers are correctly registered

	CuAssertTrue(tc, map_handler.insert				== &lhdict_insert);
	CuAssertTrue(tc, map_handler.create_dictionary	== &lhdict_create_dictionary);
	CuAssertTrue(tc, map_handler.update				== &lhdict_update);
	CuAssertTrue(tc, map_handler.remove				== &lhdict_delete);
	CuAssertTrue(tc, map_handler.delete_dictionary	== &lhdict_delete_dictionary);

}

/**
@brief 		Tests for creatation and deletion of open address hash.

@param 		tc
				CuTest
 */
void
test_linear_hash_handler_create_destroy(
	CuTest		*tc
)
{
	record_info_t record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size = 4;
	record.value_size = 10;
	int initial_size = 4;

	ion_dictionary_id_t	id = 0;

	dictionary_handler_t map_handler;			//create handler for hashmap

	lhdict_init(&map_handler);					//register handler for hashmap

	//collection handler for test collection
	dictionary_t test_dictionary;

	//register the appropriate handler for a given collection
	dictionary_create(&map_handler, &test_dictionary, key_type_numeric_signed, id, record.key_size, record.value_size, initial_size);

	CuAssertTrue(tc, (((linear_hashmap_t *)test_dictionary.instance)->super.record.key_size) == record.key_size);
	CuAssertTrue(tc, (((linear_hashmap_t *)test_dictionary.instance)->super.record.value_size) == record.value_size);
	CuAssertTrue(tc, (((linear_hashmap_t *)test_dictionary.instance)->write_concern) == wc_insert_unique);

	/** and clean up dictionary when done */
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

/**
@brief 		Tests a simple insert into dictionary and simple query

@details	Tests a simple insert into dictionary and simple query with the
			write_concern set to insert only
@param 		tc
				CuTest
 */
void
test_linear_hash_handler_simple_insert(
	CuTest		*tc
)
{

}


/**
@brief 		Tests a simple insert into dictionary and simple query

@details	Tests a simple insert into dictionary and simple query with the
		**	write_concern set to insert only
@param 		tc
				CuTest
 */
void
test_linear_hash_handler_simple_insert_and_query(
	CuTest		*tc
)
{

	record_info_t record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size = 4;
	record.value_size = 10;

	ion_dictionary_id_t id = 0;

	int test_key = 1;
	char test_value[record.value_size];

	dictionary_handler_t dict_handler;			//create handler for hashmap

	lhdict_init(&dict_handler);					//register handler for hashmap

	//collection handler for test collection
	dictionary_t test_dictionary;

	//register the appropriate handler for a given collection
	CuAssertTrue(tc, err_invalid_initial_size 	==	dictionary_create(&dict_handler, &test_dictionary, key_type_numeric_signed, id, record.key_size, record.value_size, 0));

	CuAssertTrue(tc, err_ok	==	dictionary_create(&dict_handler, &test_dictionary, key_type_numeric_signed, id,  record.key_size, record.value_size, 4));

	sprintf((char*)test_value, "value : %i", test_key);

	CuAssertTrue(tc, err_ok 	== 	test_dictionary.handler->insert(&test_dictionary,(ion_key_t)&test_key,(ion_value_t)test_value));

	ion_value_t read_value = (ion_value_t)malloc(test_dictionary.instance->record.value_size );

	CuAssertTrue(tc, err_ok	==	test_dictionary.handler->get(&test_dictionary,(ion_key_t)&test_key,(ion_value_t)read_value));

	CuAssertTrue(tc,0		== memcmp(test_value,(char *)read_value,record.value_size));

	free(read_value);

	//delete the dictionary
	CuAssertTrue(tc, err_ok == test_dictionary.handler->delete_dictionary(&test_dictionary));

	CuAssertTrue(tc, test_dictionary.instance == NULL);


}

/**
@brief 		Tests a simple delete from dictionary

@param 		tc
				CuTest
 */
void
test_linear_hash_handler_simple_delete(
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
/**
@brief		Tests that value updates are allowed with write_concern
			is set for wc_update.

@details	Tests that duplicate values are not allowed with write_concern
			is set for wc_update but will update the value.

@param 		tc
				CuTest
 */
void
test_linear_hash_handler_duplicate_insert_2(
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
test_linear_hash_handler_update_1(
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
test_linear_hash_handler_update_2(
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
test_linear_hash_handler_delete_1(
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
test_linear_hash_handler_duplicate_insert_1(
	CuTest		*tc
)
{

}

void
test_linear_hash_handler_delete_2(
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
test_linear_hash_handler_capacity(
	CuTest		*tc
)
{
}



void
test_linear_hash_dictionary_cursor_equality(
	CuTest		*tc
)
{
	int 			size;
	record_info_t 	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size 	= 4;
	record.value_size 	= 10;
	size 				= 8;

	dictionary_handler_t 	file_handler;			//create handler for hashmap
	dictionary_t			test_dictionary;		//collection handler for test collection

	/** Assuming that it is being created correctly */
	create_linear_hash_test_collection(&file_handler, &record, size, &test_dictionary, key_type_numeric_signed, size);

	dict_cursor_t 			*cursor;			//create a new cursor pointer

	//create a new predicate statement
	predicate_t *predicate;
	predicate = (predicate_t*)malloc(sizeof(predicate_t));
	predicate->type = predicate_equality;
	//need to prepare predicate correctly
	predicate->statement.equality.equality_value = (ion_key_t)malloc(sizeof(int));

	/** @Todo This needs to be improved */
	memcpy(predicate->statement.equality.equality_value,(ion_key_t)&(int){1},sizeof(int));

	//test that the query runs on collection okay
	CuAssertTrue(tc, err_ok 				== test_dictionary.handler->find(&test_dictionary, predicate, &cursor));
#if DEBUG
	DUMP(cursor->status,"%i");
#endif
	//check the status of the cursor as it should be initialized
	CuAssertTrue(tc, cs_cursor_initialized	== cursor->status);

	free(predicate->statement.equality.equality_value);
	free(predicate);

	//destroy the cursor
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}


void
test_linear_hash_dictionary_cursor_range_signed(
	CuTest		*tc
)
{
	int size;
	record_info_t record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size = 4;
	record_info.value_size = 10;
	size = 4;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary,key_type_numeric_signed, size);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type 			= predicate_range;

	//need to prepare predicate correctly
	predicate.statement.range.geq_value = (ion_key_t)malloc(sizeof(int));
	predicate.statement.range.leq_value = (ion_key_t)malloc(sizeof(int));

	memcpy(predicate.statement.range.geq_value,(ion_key_t)&(int){1},sizeof(int));
	memcpy(predicate.statement.range.leq_value,(ion_key_t)&(int){6},sizeof(int));

	//test that the query runs on collection okay
	CuAssertTrue(tc, err_ok 				== test_dictionary.handler->find(&test_dictionary, &predicate, &cursor));
#if DEBUG
	DUMP(cursor->status,"%i");
#endif
	//check the status of the cursor as it should be initialized
	CuAssertTrue(tc, cs_cursor_initialized	== cursor->status);

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);

	//destroy cursor for cleanup
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);

}

void
test_linear_hash_dictionary_cursor_range_signed_2(
	CuTest		*tc
)
{
	int size;
	record_info_t record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size = 4;
	record_info.value_size = 10;
	size = 4;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary,key_type_numeric_signed, size);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type 			= predicate_range;

	//need to prepare predicate correctly
	predicate.statement.range.geq_value = (ion_key_t)malloc(sizeof(int));
	predicate.statement.range.leq_value = (ion_key_t)malloc(sizeof(int));

	memcpy(predicate.statement.range.geq_value,(ion_key_t)&(int){10},sizeof(int));
	memcpy(predicate.statement.range.leq_value,(ion_key_t)&(int){13},sizeof(int));

	//test that the query runs on collection okay
	CuAssertTrue(tc, err_ok 				== test_dictionary.handler->find(&test_dictionary, &predicate, &cursor));
#if DEBUG
	DUMP(cursor->status,"%i");
#endif
	//check the status of the cursor as it should be initialized
	CuAssertTrue(tc, cs_end_of_results	== cursor->status);

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);

	//destroy cursor for cleanup
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);

}

void
test_linear_hash_dictionary_cursor_range_signed_3(
	CuTest		*tc
)
{
	int size;
	record_info_t record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size = 4;
	record_info.value_size = 10;
	size = 2;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary,key_type_numeric_signed, size*4);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type 			= predicate_range;

	//need to prepare predicate correctly
	predicate.statement.range.geq_value = (ion_key_t)malloc(sizeof(int));
	predicate.statement.range.leq_value = (ion_key_t)malloc(sizeof(int));

	memcpy(predicate.statement.range.geq_value,(ion_key_t)&(int){10},sizeof(int));
	memcpy(predicate.statement.range.leq_value,(ion_key_t)&(int){13},sizeof(int));

	//test that the query runs on collection okay
	CuAssertTrue(tc, err_ok 				== test_dictionary.handler->find(&test_dictionary, &predicate, &cursor));
#if DEBUG
	DUMP(cursor->status,"%i");
#endif
	//check the status of the cursor as it should be initialized
	CuAssertTrue(tc, cs_end_of_results	== cursor->status);

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);

	//destroy cursor for cleanup
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);

}

void
test_linear_hash_dictionary_cursor_range_signed_4(
	CuTest		*tc
)
{
	int size;
	record_info_t record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size = 4;
	record_info.value_size = 10;
	size = 2;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary,key_type_numeric_signed, size*4);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type 			= predicate_range;

	//need to prepare predicate correctly
	predicate.statement.range.geq_value = (ion_key_t)malloc(sizeof(int));
	predicate.statement.range.leq_value = (ion_key_t)malloc(sizeof(int));

	/** ensures that value is in overflow page */
	memcpy(predicate.statement.range.geq_value,(ion_key_t)&(int){5},sizeof(int));
	memcpy(predicate.statement.range.leq_value,(ion_key_t)&(int){10},sizeof(int));

	//test that the query runs on collection okay
	CuAssertTrue(tc, err_ok 				== test_dictionary.handler->find(&test_dictionary, &predicate, &cursor));
#if DEBUG
	DUMP(cursor->status,"%i");
#endif
	//check the status of the cursor as it should be initialized
	CuAssertTrue(tc, cs_cursor_initialized	== cursor->status);

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);

	//destroy cursor for cleanup
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);

}

void
test_linear_hash_dictionary_handler_query_with_results(
	CuTest		*tc
)
{
	int size;
	record_info_t record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size = 4;
	record_info.value_size = 10;
	size = 8;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed, size);

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
	record.value 			= (ion_value_t)malloc(sizeof(ion_value_t)*record_info.value_size);
	record.key 				= (ion_key_t)malloc(sizeof(ion_value_t)*record_info.key_size);

	CuAssertTrue(tc, cs_cursor_active		== cursor->next(cursor, &record));

	//check that value is correct that has been returned
	ion_value_t				str;
	str 					= (ion_value_t)malloc(sizeof(ion_value_t)*record_info.value_size);
	sprintf((char*)str,"value : %i ", *(int *)predicate.statement.equality.equality_value);

	CuAssertTrue(tc, IS_EQUAL				== memcmp(record.value, str, record_info.value_size));

	free(str);

	//and as there is only 1 result, the next call should return empty
	CuAssertTrue(tc, cs_end_of_results		== cursor->next(cursor, &record));

	//and as there is only 1 result, the next call should return empty
	CuAssertTrue(tc, cs_end_of_results		== cursor->next(cursor, &record));

	//free up the correct predicate
	free(predicate.statement.equality.equality_value);
	free(record.value);
	free(record.key);
	//destory cursor for cleanup
	cursor->destroy(&cursor);
	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_linear_hash_dictionary_handler_query_no_results(
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

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed,size);

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
	record.key 				= (ion_key_t)malloc(sizeof(ion_key_t)*record_info.value_size);
	record.value 			= (ion_value_t)malloc(sizeof(ion_value_t)*record_info.value_size);

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
test_linear_hash_dictionary_predicate_equality(
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
	size = 4;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed, size);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	cursor = (dict_cursor_t *)((lhdict_cursor_t*)malloc(sizeof(lhdict_cursor_t)));			/** Make sure to malloc correct size */

	/** @FIMXE */
	cursor->destroy 		= lhdict_destroy_cursor;

	//create a new predicate statement
	/** IMPORTANT - This must be put on the heap as the destory method will attempt to free */
	cursor->predicate 		= (predicate_t *)malloc(sizeof(predicate_t));
	cursor->predicate->type	= predicate_equality;

	//predicate_t 			*predicate = (predicate_t *)malloc(sizeof(predicate_t));
	//predicate->type 		= predicate_equality;

	//need to prepare predicate correctly
	//predicate->statement.equality.equality_value = (ion_key_t)malloc(sizeof(int));
	cursor->predicate->statement.equality.equality_value = (ion_key_t)malloc(sizeof(int));

	//memcpy(predicate->statement.equality.equality_value,(ion_key_t)&(int){1},sizeof(int));
	memcpy(cursor->predicate->statement.equality.equality_value,(ion_key_t)&(int){1},sizeof(int));

	cursor->dictionary 		= &test_dictionary;				//register test dictionary
	//cursor->predicate 		= predicate;					//register predicate
	cursor->type			= cursor_equality;

	((lhdict_cursor_t*)cursor)->overflow = NULL;

	io_printf("test 1\n");
	memcpy(key_under_test,(ion_key_t)&(int){1},sizeof(int));

	CuAssertTrue(tc, IS_EQUAL 	== lhdict_test_predicate(cursor, key_under_test));
	io_printf("test 2\n");
	memcpy(key_under_test,(ion_key_t)&(int){2},sizeof(int));

	CuAssertTrue(tc, IS_EQUAL 	!= lhdict_test_predicate(cursor, key_under_test));
	io_printf("test 3\n");
	memcpy(key_under_test,(ion_key_t)&(int){-1},sizeof(int));

	CuAssertTrue(tc, IS_EQUAL 	!= lhdict_test_predicate(cursor, key_under_test));

	io_printf("freeing key\n");
	free(key_under_test);

	/** @FIXME this does not need to be done */
	//free up the correct predicate
	//io_printf("freeing predicate\n");
	//DUMP(cursor->predicate->statement.equality.equality_value,"%p");
	//free(cursor->predicate->statement.equality.equality_value);

	//predicate->statement.equality.equality_value = NULL;

	//destroy cursor for cleanup
	io_printf("destroying cursor\n");
	cursor->destroy(&cursor);
	//and destroy the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_linear_hash_dictionary_predicate_range_signed(
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
	size = 4;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary,key_type_numeric_signed,size);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	cursor = (dict_cursor_t *)malloc(sizeof(lhdict_cursor_t));

	/** @FIXME */
	cursor->destroy 		= lhdict_destroy_cursor;

	((lhdict_cursor_t*)cursor)->overflow = NULL;

	//create a new predicate statement
	predicate_t 			*predicate = (predicate_t *)malloc(sizeof(predicate_t));
	predicate->type 			= predicate_range;

	//need to prepare predicate correctly
	predicate->statement.range.geq_value = (ion_key_t)malloc(sizeof(int));
	predicate->statement.range.leq_value = (ion_key_t)malloc(sizeof(int));

	memcpy(predicate->statement.range.geq_value,(ion_key_t)&(int){-1},sizeof(int));
	memcpy(predicate->statement.range.leq_value,(ion_key_t)&(int){1},sizeof(int));

	cursor->dictionary 		= &test_dictionary;				//register test dictionary
	cursor->predicate 		= predicate;					//register predicate
	cursor->type			= cursor_range;

	memcpy(key_under_test,(ion_key_t)&(int){0},sizeof(int));

	CuAssertTrue(tc, IS_EQUAL 	== lhdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){-1},sizeof(int));

	CuAssertTrue(tc, IS_EQUAL == lhdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){1},sizeof(int));

	CuAssertTrue(tc, IS_EQUAL == lhdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){2},sizeof(int));

	CuAssertTrue(tc, IS_GREATER == lhdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(int){-2},sizeof(int));

	CuAssertTrue(tc, IS_LESS 	== lhdict_test_predicate(cursor, key_under_test));

	free(key_under_test);

	//destroy cursor for cleanup
	cursor->destroy(&cursor);
	//and destroy the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}
void
test_linear_hash_dictionary_predicate_range_unsigned(
	CuTest		*tc
)
{
	ion_key_t key_under_test;
	key_under_test = (ion_key_t)malloc(sizeof(unsigned int));

	int size;
	record_info_t record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size = 4;
	record_info.value_size = 10;
	size = 4;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t 			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary,key_type_numeric_unsigned, size);

	dict_cursor_t 			*cursor;				//create a new cursor pointer

	cursor = (dict_cursor_t *)malloc(sizeof(lhdict_cursor_t));
	/** @FIXME */
	cursor->destroy 		= lhdict_destroy_cursor;

	((lhdict_cursor_t*)cursor)->overflow = NULL;

	//create a new predicate statement
	predicate_t 			*predicate = (predicate_t *)malloc(sizeof(predicate_t));
	predicate->type 		= predicate_range;

	//need to prepare predicate correctly
	predicate->statement.range.geq_value = (ion_key_t)malloc(sizeof(unsigned int));
	predicate->statement.range.leq_value = (ion_key_t)malloc(sizeof(unsigned int));

	memcpy(predicate->statement.range.geq_value,(ion_key_t)&(unsigned int){0},sizeof(unsigned int));
	memcpy(predicate->statement.range.leq_value,(ion_key_t)&(unsigned int){2},sizeof(unsigned int));

	cursor->dictionary 		= &test_dictionary;				//register test dictionary
	cursor->predicate 		= predicate;					//register predicate
	cursor->type			= cursor_range;

	memcpy(key_under_test,(ion_key_t)&(unsigned int){0},sizeof(unsigned int));

	CuAssertTrue(tc, IS_EQUAL 	== lhdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){1},sizeof(unsigned int));

	CuAssertTrue(tc, IS_EQUAL 	== lhdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){2},sizeof(unsigned int));

	CuAssertTrue(tc, IS_EQUAL 	== lhdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){3},sizeof(unsigned int));

	CuAssertTrue(tc, IS_GREATER 	== lhdict_test_predicate(cursor, key_under_test));

	memcpy(key_under_test,(ion_key_t)&(unsigned int){4},sizeof(unsigned int));

	CuAssertTrue(tc, IS_GREATER 	== lhdict_test_predicate(cursor, key_under_test));

	free(key_under_test);

	//destroy cursor for cleanup
	cursor->destroy(&cursor);
	//and destroy the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

void
test_linear_hash_dictionary_cursor_range(
	CuTest		*tc
)
{
	int 			size;
	record_info_t 	record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size 	= 4;
	record_info.value_size 	= 10;
	size 					= 8;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed,size);

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
	ion_record_t 					record;
	record.key 						= (ion_key_t)malloc(record_info.key_size);
	record.value 					= (ion_value_t)malloc(record_info.value_size);

	int result_count = 0;
	status_t cursor_status;

	while( cs_cursor_active == (cursor_status = cursor->next(cursor, &record)))
	{

		CuAssertTrue(tc, cs_cursor_active		== cursor_status);

		//check that value is correct that has been returned
		ion_value_t	str;
		str = (ion_value_t)malloc(record_info.value_size + 1);			/** make sure to include null */
		sprintf((char*)str,"value : %i ", (*(int *)predicate.statement.range.geq_value) + result_count);

		CuAssertTrue(tc, IS_EQUAL				== memcmp(record.value, str, record_info.value_size));
		//printf("result %s\n",(char*)str);
		result_count++;
		free(str);
	}
	CuAssertTrue(tc, 5						== result_count);

	//and as there is only 1 result, the next call should return empty
	CuAssertTrue(tc, cs_end_of_results		== cursor->next(cursor, &record));

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);

	//free up record
	free(record.key);
	free(record.value);

	//destroy the cursor
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

/**
 * @brief Tests cursor wrapping using only primary pages with no splitting
 * @param tc
 */
void
test_linear_hash_dictionary_cursor_range_2(
	CuTest		*tc
)
{
	int 			size;
	record_info_t 	record_info;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size 	= 4;
	record_info.value_size 	= 10;
	size 					= 4;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed,2*size);

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
	ion_record_t 					record;
	record.key 						= (ion_key_t)malloc(record_info.key_size);
	record.value 					= (ion_value_t)malloc(record_info.value_size);

	int result_count = 0;
	status_t cursor_status;
	int expected_results[] = {1,5,2,3,4};
	while( cs_cursor_active == (cursor_status = cursor->next(cursor, &record)))
	{

		CuAssertTrue(tc, cs_cursor_active		== cursor_status);

		//check that value is correct that has been returned
		ion_value_t	str;
		str = (ion_value_t)malloc(record_info.value_size + 1);
		sprintf((char*)str,"value : %i ", expected_results[result_count]);

		CuAssertTrue(tc, IS_EQUAL				== memcmp(record.value, str, record_info.value_size));
#if DEBUG
		io_printf("result:%i  %s\n",*(int*)record.key,(char*)record.value);
#endif
		result_count++;
		free(str);
	}
	CuAssertTrue(tc, 5						== result_count);

	//and as there is only 1 result, the next call should return empty
	CuAssertTrue(tc, cs_end_of_results		== cursor->next(cursor, &record));

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);

	//free up record
	free(record.key);
	free(record.value);

	//destroy the cursor
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

/**
 * @brief Tests cursor wrapping with overflow pages
 * @param tc
 */
void
test_linear_hash_dictionary_cursor_range_3(
	CuTest		*tc
)
{
	int 			size;
	record_info_t 	record_info;
	int 			num_keys;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size 	= 4;
	record_info.value_size 	= 10;
	size 					= 2;
	num_keys				= 8;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed,num_keys);

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
	ion_record_t 					record;
	record.key 						= (ion_key_t)malloc(record_info.key_size);
	record.value 					= (ion_value_t)malloc(record_info.value_size);

	int result_count = 0;
	status_t cursor_status;
	int expected_results[] = {1,3,5,2,4};

	while( cs_cursor_active == (cursor_status = cursor->next(cursor, &record)))
	{

		CuAssertTrue(tc, cs_cursor_active		== cursor_status);

		//check that value is correct that has been returned
		ion_value_t	str;
		str = (ion_value_t)malloc(record_info.value_size + 1);
		sprintf((char*)str,"value : %i ", expected_results[result_count]);
//		io_printf("** result1:%i  %s\n",*(int*)record.key,(char*)record.value);
//		io_printf("** result2:%i  %s\n",*(int*)record.key,str);

		CuAssertTrue(tc, IS_EQUAL				== memcmp(record.value, str, record_info.value_size));
#if DEBUG
		io_printf("** result:%i  %s\n",*(int*)record.key,(char*)record.value);
#endif
		result_count++;
		free(str);
	}
	CuAssertTrue(tc, 5						== result_count);

	//and as there is only 1 result, the next call should return empty
	CuAssertTrue(tc, cs_end_of_results		== cursor->next(cursor, &record));

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);

	//free up record
	free(record.key);
	free(record.value);

	//destroy the cursor
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

/**
 * @brief Tests cursor wrapping with overflow pages and ensures that
 * the initial cursor setup will be forced to wrap.
 * @param tc
 */
void
test_linear_hash_dictionary_cursor_range_4(
	CuTest		*tc
)
{
	int 			size;
	record_info_t 	record_info;
	int 			num_keys;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size 	= 4;
	record_info.value_size 	= 10;
	size 					= 4;
	num_keys				= 8;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed,num_keys);

	dict_cursor_t 			*cursor;			//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type 			= predicate_range;
	//need to prepare predicate correctly
	predicate.statement.range.geq_value = (ion_key_t)malloc(sizeof(int));

	memcpy(predicate.statement.range.geq_value,(ion_key_t)&(int){2},sizeof(int));

	predicate.statement.range.leq_value = (ion_key_t)malloc(sizeof(int));

	memcpy(predicate.statement.range.leq_value,(ion_key_t)&(int){5},sizeof(int));

	test_dictionary.handler->remove(&test_dictionary,(ion_key_t)&(int){2});
	test_dictionary.handler->remove(&test_dictionary,(ion_key_t)&(int){3});


	//test that the query runs on collection okay
	CuAssertTrue(tc, err_ok 				== test_dictionary.handler->find(&test_dictionary, &predicate, &cursor));

	//check the status of the cursor as it should be initialized
	CuAssertTrue(tc, cs_cursor_initialized	== cursor->status);

	//user must allocate memory before calling next()
	ion_record_t 					record;
	record.key 						= (ion_key_t)malloc(record_info.key_size);
	record.value 					= (ion_value_t)malloc(record_info.value_size);

	int result_count = 0;
	status_t cursor_status;
	int expected_results[] = {4,5};
	while( cs_cursor_active == (cursor_status = cursor->next(cursor, &record)))
	{

		CuAssertTrue(tc, cs_cursor_active		== cursor_status);

		//check that value is correct that has been returned
		ion_value_t	str;
		str = (ion_value_t)malloc(record_info.value_size + 1);
		sprintf((char*)str,"value : %i ", expected_results[result_count]);

		CuAssertTrue(tc, IS_EQUAL				== memcmp(record.value, str, record_info.value_size));

		result_count++;
		free(str);
	}
	CuAssertTrue(tc, 2						== result_count);

	//and as there is only 1 result, the next call should return empty
	CuAssertTrue(tc, cs_end_of_results		== cursor->next(cursor, &record));

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);

	//free up record
	free(record.key);
	free(record.value);

	//destroy the cursor
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

/**
 * @brief Tests cursor wrapping with overflow pages and ensures that
 * the initial cursor setup will be forced to wrap and scan into overflow
 * page.
 * @param tc
 */
void
test_linear_hash_dictionary_cursor_range_5(
	CuTest		*tc
)
{
	int 			size;
	record_info_t 	record_info;
	int 			num_keys;

	/* this is required for initializing the hash map and should come from the dictionary */
	record_info.key_size 	= 4;
	record_info.value_size 	= 10;
	size 					= 2;
	num_keys				= 8;

	dictionary_handler_t 	map_handler;			//create handler for hashmap
	dictionary_t			test_dictionary;		//collection handler for test collection

	create_linear_hash_test_collection(&map_handler, &record_info, size, &test_dictionary, key_type_numeric_signed,num_keys);

	dict_cursor_t 			*cursor;			//create a new cursor pointer

	//create a new predicate statement
	predicate_t 			predicate;
	predicate.type = predicate_range;
	//need to prepare predicate correctly
	predicate.statement.range.geq_value = (ion_key_t)malloc(sizeof(int));

	memcpy(predicate.statement.range.geq_value,(ion_key_t)&(int){2},sizeof(int));

	predicate.statement.range.leq_value = (ion_key_t)malloc(sizeof(int));

	memcpy(predicate.statement.range.leq_value,(ion_key_t)&(int){5},sizeof(int));

	test_dictionary.handler->remove(&test_dictionary,(ion_key_t)&(int){2});
	test_dictionary.handler->remove(&test_dictionary,(ion_key_t)&(int){3});

	//test that the query runs on collection okay
	CuAssertTrue(tc, err_ok 				== test_dictionary.handler->find(&test_dictionary, &predicate, &cursor));

	//check the status of the cursor as it should be initialized
	CuAssertTrue(tc, cs_cursor_initialized	== cursor->status);

	//user must allocate memory before calling next()
	ion_record_t 					record;
	record.key 						= (ion_key_t)malloc(record_info.key_size);
	record.value 					= (ion_value_t)malloc(record_info.value_size);

	int result_count = 0;
	status_t cursor_status;
	int expected_results[] = {4,5};
	while( cs_cursor_active == (cursor_status = cursor->next(cursor, &record)))
	{

		CuAssertTrue(tc, cs_cursor_active		== cursor_status);

		//check that value is correct that has been returned
		ion_value_t	str;
		str = (ion_value_t)malloc(record_info.value_size + 1);
		sprintf((char*)str,"value : %i ", expected_results[result_count]);

		CuAssertTrue(tc, IS_EQUAL				== memcmp(record.value, str, record_info.value_size));

		result_count++;
		free(str);
	}
	CuAssertTrue(tc, 2						== result_count);

	//and as there is only 1 result, the next call should return empty
	CuAssertTrue(tc, cs_end_of_results		== cursor->next(cursor, &record));

	//free up the correct predicate
	free(predicate.statement.range.geq_value);
	free(predicate.statement.range.leq_value);

	//free up record
	free(record.key);
	free(record.value);

	//destroy the cursor
	cursor->destroy(&cursor);

	//and check that cursor has been destroyed correctly
	CuAssertTrue(tc, NULL 					== cursor);

	//and destory the collection
	test_dictionary.handler->delete_dictionary(&test_dictionary);
}

CuSuite*
linear_hash_handler_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_linear_hash_handler_function_registration);
	SUITE_ADD_TEST(suite, test_linear_hash_handler_create_destroy);
	SUITE_ADD_TEST(suite, test_linear_hash_handler_simple_insert_and_query);
	/** @tdodo need simple query*/
	/*SUITE_ADD_TEST(suite, test_linear_hash_handler_simple_delete);
	SUITE_ADD_TEST(suite, test_linear_hash_handler_duplicate_insert_1);
	SUITE_ADD_TEST(suite, test_linear_hash_handler_duplicate_insert_2);
	SUITE_ADD_TEST(suite, test_linear_hash_handler_update_1);
	SUITE_ADD_TEST(suite, test_linear_hash_handler_update_2);
	SUITE_ADD_TEST(suite, test_linear_hash_handler_delete_1);
	SUITE_ADD_TEST(suite, test_linear_hash_handler_delete_2);
	SUITE_ADD_TEST(suite, test_linear_hash_handler_capacity);*/
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_predicate_equality);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_predicate_range_signed);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_predicate_range_unsigned);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_cursor_equality);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_cursor_range_signed);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_cursor_range_signed_2);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_cursor_range_signed_3);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_cursor_range_signed_4);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_handler_query_with_results);
	//
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_handler_query_no_results);
	//
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_cursor_range);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_cursor_range_2);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_cursor_range_3);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_cursor_range_4);
	SUITE_ADD_TEST(suite, test_linear_hash_dictionary_cursor_range_5);
	return suite;
}

void
runalltests_linear_hash_handler()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= linear_hash_handler_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}


