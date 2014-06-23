/**
@file 		oadictionaryhandler.c

@author		Scott Ronald Fazackerley

 */

#include <stdio.h>
#include <string.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/openaddresshash/oahash.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"
#include "./../../../../dictionary/openaddresshash/oadictionaryhandler.h"

#define MAX_HASH_TEST 100

/**
@brief		Validates the correct registration of function pointers
			for open address dictionary structure.

@param 		tc
				CuTest
 */
void
test_open_address_hashmap_handler_function_registration(
	CuTest		*tc
)
{
	dictionary_handler_t map_handler;			//create handler for hashmap

	oadict_init(&map_handler);					//register handler for hashmap

	//check to ensure that function pointers are correctly registered
	CuAssertTrue(tc, map_handler.insert				== &oadict_insert);
	CuAssertTrue(tc, map_handler.create_dictionary	== &oadict_create_dictionary);
	CuAssertTrue(tc, map_handler.update				== &oadict_update);
	CuAssertTrue(tc, map_handler.delete				== &oadict_delete);
	CuAssertTrue(tc, map_handler.delete_dictionary	== &oadict_delete_dictionary);

}

/**
@brief 		Tests for creatation and deletion of open address hash.

@param 		tc
				CuTest
 */
void
test_open_address_hashmap_handler_create_destroy(
	CuTest		*tc
)
{

	int size;
	record_t record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size = 4;
	record.value_size = 10;
	size = 10;

	dictionary_handler_t map_handler;			//create handler for hashmap

	oadict_init(&map_handler);					//register handler for hashmap

	//collection handler for test collection
	dictionary_t test_dictionary;

	//register the appropriate handler for a given collection
	dictionary_create(&map_handler, &test_dictionary, record.key_size, record.value_size,size);

	CuAssertTrue(tc, (((hashmap_t *)test_dictionary.instance)->record.key_size) == record.key_size);
	CuAssertTrue(tc, (((hashmap_t *)test_dictionary.instance)->record.value_size) == record.value_size);
	CuAssertTrue(tc, (((hashmap_t *)test_dictionary.instance)->map_size) == size);
	CuAssertTrue(tc, (((hashmap_t *)test_dictionary.instance)->compute_hash) == &oah_compute_simple_hash);
	CuAssertTrue(tc, (((hashmap_t *)test_dictionary.instance)->write_concern) == wc_insert_unique);
	CuAssertTrue(tc, test_dictionary.handler->delete_dictionary(&test_dictionary) == err_ok);
	CuAssertTrue(tc, test_dictionary.instance == NULL);

	//todo fix free value status

/*&
	io_printf("Insert\n");

		char value[record.value_size];

	int i;

	for (i = 0; i < MAX_HASH_TEST; i++)
	{
		printf("%i ", i);
		printf("%i\n", oah_compute_simple_hash(hash_map, (char *)(&i), sizeof(int)));

	}*/

	/*
	status = memchunk_create_segment(&segment, numitems, itemsize);

	CuAssertTrue(tc, IS_STATUS_OK(status));

	status = memchunk_init_handler(&handler, segment, numitems, itemsize);
	CuAssertTrue(tc, IS_STATUS_OK(status));

	CuAssertTrue(tc, numitems		== handler.super.numitems);
	CuAssertTrue(tc, itemsize		== handler.super.itemsize);
	CuAssertTrue(tc, CHUNKTYPE_INMEMORY	== handler.super.type);
	CuAssertTrue(tc, NULL			!= handler.segment);

	status = memchunk_destroy_segment(&(handler.segment));
	CuAssertTrue(tc, IS_STATUS_OK(status));
	CuAssertTrue(tc, NULL			== handler.segment);*/
}

/**
@brief 		Tests a simple insert into dictionary and simple query

@details	Tests a simple insert into dictionary and simple query with the
			write_concern set to insert only
@param 		tc
				CuTest
 */
void
test_open_address_hashmap_handler_simple_insert_and_query(
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
test_open_address_hashmap_handler_simple_delete(
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
test_open_address_hashmap_handler_duplicate_insert_1(
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
test_open_address_hashmap_handler_duplicate_insert_2(
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
test_open_address_hashmap_handler_update_1(
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
test_open_address_hashmap_handler_update_2(
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
test_open_address_hashmap_handler_delete_1(
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
test_open_address_hashmap_handler_delete_2(
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
test_open_address_hashmap_handler_capacity(
	CuTest		*tc
)
{
}

CuSuite*
open_address_hashmap_handler_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_open_address_hashmap_handler_function_registration);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_handler_create_destroy);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_handler_simple_insert_and_query);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_handler_simple_delete);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_handler_duplicate_insert_1);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_handler_duplicate_insert_2);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_handler_update_1);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_handler_update_2);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_handler_delete_1);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_handler_delete_2);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_handler_capacity);

	return suite;
}

void
runalltests_open_address_hash_handler()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= open_address_hashmap_handler_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}
