/**
@file		oahash.c
@author		Scott Ronald Fazackerley

@details	Tests base operations for open address hash map
 */


#include <stdio.h>
#include <string.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/openaddresshash/oahash.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"

#define MAX_HASH_TEST 100
#define STD_MAP_SIZE 10

void
initialize_hash_map(
	int			size,
	record_t	*record,
	hashmap_t 	*map
)
{
	oah_initialize(map, oah_compute_simple_hash, record->key_size, record->value_size, size);
}

void
initialize_hash_map_std_conditions(
	hashmap_t	*map
)
{


	record_t record;
	record.key_size = 4;
	record.value_size = 10;
	initialize_hash_map(STD_MAP_SIZE, &record, map);
}

/**
@brief 		Tests for creation and deletion of open address hash.

@param 		tc
				CuTest
 */
void
test_open_address_hashmap_initialize(
	CuTest		*tc
)
{

	/* this is required for initializing the hash map and should come from the dictionary */
	int size;
	record_t record;
	record.key_size = 4;
	record.value_size = 10;
	size = 10;
	hashmap_t map;
	initialize_hash_map(size, &record, &map);

	//valid correct map settings
	CuAssertTrue(tc, map.record.key_size 		== record.key_size);
	CuAssertTrue(tc, map.record.value_size		== record.value_size);
	CuAssertTrue(tc, map.map_size 				== size);
	CuAssertTrue(tc, map.compute_hash 			== &oah_compute_simple_hash);
	CuAssertTrue(tc, map.write_concern 			== wc_insert_unique);

}

/**
@brief		Tests the computation of a simple hash value

@param 		tc
				CuTest
 */
void
test_open_address_hashmap_compute_simple_hash(
	CuTest		*tc
)
{

	hashmap_t map;			//create handler for hashmap
	int i;

	initialize_hash_map_std_conditions(&map);

	for (i = 0; i< MAX_HASH_TEST; i++)
	{
		CuAssertTrue(tc, (i % map.map_size) ==
			oah_compute_simple_hash(&map, (ion_key_t)((int *)&i), sizeof(i)));
	}
}

/**
@brief 		Test locating an element in the hashmap and returns the theoretical
			location of the item.

@param 		tc
				CuTest
 */
void
test_open_address_hashmap_get_location(
	CuTest		*tc
)
{
	int i;

	for (i = 0; i< MAX_HASH_TEST; i++)
	{
		CuAssertTrue(tc, (i % STD_MAP_SIZE) == oah_get_location((hash_t)i, STD_MAP_SIZE));
	}
}

/**
@brief		Test locating an element in the hashmap and returns the physical
			address of the item.

@details	Test locating an element in the hashmap and returns the physical
			address of the item.  May probe multiple locations depending of
			current occupancy of hashmap

@param 		tc
				CuTest
 */

void
test_open_address_hashmap_find_item_location(
	CuTest		*tc
)
{

}


/**
@brief 		Tests a simple insert into dictionary and simple query

@details	Tests a simple insert into dictionary and simple query with the
			write_concern set to insert only
@param 		tc
				CuTest
 */
void
test_open_address_hashmap_simple_insert_and_query(
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
test_open_address_hashmap_simple_delete(
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
test_open_address_hashmap_duplicate_insert_1(
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
test_open_address_hashmap_duplicate_insert_2(
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
test_open_address_hashmap_update_1(
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
test_open_address_hashmap_update_2(
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
test_open_address_hashmap_delete_1(
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
test_open_address_hashmap_delete_2(
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
test_open_address_hashmap_capacity(
	CuTest		*tc
)
{
}

CuSuite*
open_address_hashmap_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_open_address_hashmap_initialize);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_compute_simple_hash);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_get_location);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_find_item_location);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_simple_insert_and_query);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_simple_delete);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_duplicate_insert_1);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_duplicate_insert_2);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_update_1);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_update_2);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_delete_1);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_delete_2);
	SUITE_ADD_TEST(suite, test_open_address_hashmap_capacity);

	return suite;
}

void
runalltests_open_address_hash()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= open_address_hashmap_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}
