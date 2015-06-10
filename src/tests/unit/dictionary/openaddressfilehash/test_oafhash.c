/**
@file		oahash.c
@author		Scott Ronald Fazackerley

@details	Tests base operations for open address hash map
 */


#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/openaddressfilehash/oafhash.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"

#define MAX_HASH_TEST 100
#define STD_MAP_SIZE 10
#define TEST_FILE	"file.bin"


/**
@brief			Helper function to visualize hashmap contents

@param 			map
					The hashmap to visualize.
 */
void
check_file_map(
	file_hashmap_t 	*map
)
{
	int i;
	int bucket_size = map->super.record.key_size + map->super.record.value_size + sizeof(char);

	rewind(map->file);
	hash_bucket_t * record;
	record = (hash_bucket_t *)malloc(bucket_size);
	for (i = 0; i < map->map_size; i++)
	{
		DUMP(i,"%i");
		DUMP(map->map_size,"%i");
		int j;
		DUMP((int) fread(record,bucket_size,1,map->file),"%d");
		printf("reading\n");
		fflush(stdout);
		for (j = 0; j < bucket_size; j++)
		{
			printf("%c ", *((char *)record) + j);
		}
		printf("\n");
		fflush(stdout);
	}
	//and now check key positions
}
void
initialize_file_hash_map(
	int			size,
	record_info_t	*record,
	file_hashmap_t 	*map
)
{
	oafh_initialize(map, oafh_compute_simple_hash, /*dictionary_compare_signed_value,*/ map->super.key_type, record->key_size, record->value_size, size);
	map->super.compare = dictionary_compare_signed_value;
}

void
initialize_file_hash_map_std_conditions(
	file_hashmap_t	*map
)
{

	record_info_t record;
	record.key_size 		= 4;
	record.value_size 		= 10;
	map->super.key_type 	= key_type_numeric_signed;
	initialize_file_hash_map(STD_MAP_SIZE, &record, map);

}

/**
@brief 		Tests for creation and deletion of open address hash.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_initialize(
	CuTest		*tc
)
{

	/* this is required for initializing the hash map and should come from the dictionary */
	int size;
	record_info_t record;
	record.key_size = 4;
	record.value_size = 10;
	size = 10;
	file_hashmap_t map;
	map.super.key_type = key_type_numeric_signed;			//default to use int key type

	initialize_file_hash_map(size, &record, &map);

	//valid correct map settings
	CuAssertTrue(tc, map.super.record.key_size 		== record.key_size);
	CuAssertTrue(tc, map.super.record.value_size	== record.value_size);
	CuAssertTrue(tc, map.map_size 					== size);
	CuAssertTrue(tc, map.compute_hash 				== &oafh_compute_simple_hash);
	CuAssertTrue(tc, map.write_concern 				== wc_insert_unique);

	fclose(map.file);
	remove(TEST_FILE);
}

/**
@brief		Tests the computation of a simple hash value

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_compute_simple_hash(
	CuTest		*tc
)
{

	file_hashmap_t map;			//create handler for hashmap
	int i;

	initialize_file_hash_map_std_conditions(&map);

	for (i = 0; i< MAX_HASH_TEST; i++)
	{
		CuAssertTrue(tc, (i % map.map_size) ==
			oafh_compute_simple_hash(&map, (ion_key_t)((int *)&i), sizeof(i)));
	}
}

/**
@brief 		Test locating an element in the hashmap and returns the theoretical
			location of the item.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_get_location(
	CuTest		*tc
)
{
	int i;

	for (i = 0; i< MAX_HASH_TEST; i++)
	{
		CuAssertTrue(tc, (i % STD_MAP_SIZE) == oafh_get_location((hash_t)i, STD_MAP_SIZE));
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
test_open_address_file_hashmap_find_item_location(
	CuTest		*tc
)
{
	file_hashmap_t map;			//create handler for hashmap
	int i;
	int offset;

	initialize_file_hash_map_std_conditions(&map);

	/** Manually populate records */
	record_info_t record 			= map.super.record;

	char *item;

	//manually map out item stucture
	item = (char *)malloc(sizeof(char) * (record.key_size + record.value_size +sizeof(char)));

	//manually populate array
	hash_bucket_t *item_ptr 	= (hash_bucket_t *)item;

	//FILE *file_ptr 				= fopen("test.bin","w+");
	//map.file 					= file_ptr;

	int bucket_size 			= sizeof(char) + record.key_size
									+ record.value_size;

	//rewind
	rewind(map.file);

	for (offset = 0; offset < map.map_size; offset ++)
	{
		// apply continual offsets
		//pos_ptr 				= (map.entry
		//							+ (offset*bucket_size)%(map.map_size*bucket_size));

		//printf("writing to %i\n",(offset*bucket_size)%(map.map_size*bucket_size));

		fseek(map.file,(offset*bucket_size)%(map.map_size*bucket_size),SEEK_SET);

		for (i = 0; i<map.map_size; i++)
		{
			item_ptr->status 	= IN_USE;
			memcpy (item_ptr->data, (int *)&i, sizeof(int));
			char str[10];
			//build up the value
			sprintf(str,"%02i is key",i);
			//and copy it directly into the slot
			memcpy((item_ptr->data + sizeof(int)), str, 10);

			//memcpy(pos_ptr, item_ptr, bucket_size);

			fwrite(item_ptr, bucket_size, 1, map.file);
			//printf("Moving to position %i\n", ((((i+1+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size)));
			//pos_ptr = map.entry + ((((i+1+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size));
			fseek(map.file,((((i+1+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size)),SEEK_SET);
			//printf("current file pos: %i\n",(int)	ftell(map.file));
		}

		//and now check key positions
		for (i = 0; i<map.map_size; i++)
		{
			int location;
			CuAssertTrue(tc, err_ok	== oafh_find_item_loc(&map, (ion_key_t)(&i), &location));
			//printf("location %i\n",location);
			CuAssertTrue(tc, (i+offset)%map.map_size == location);
		}
	}

	free(item);
	fclose(map.file);
	remove(TEST_FILE);

}

/**
@brief 		Tests a simple insert into map and reads results directly from map

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_simple_insert(
	CuTest		*tc
)
{
	file_hashmap_t map;			//create handler for hashmap
	int i;
	int offset = 0;

	initialize_file_hash_map_std_conditions(&map);

	//check_file_map(&map);

	// Manually populate records
	record_info_t record 			= map.super.record;

	int bucket_size 			= sizeof(char)
									+ record.key_size + record.value_size;
	//rewind
	rewind(map.file);

	for (offset = 0; offset < map.map_size; offset ++)
	{

		for (i = 0; i<map.map_size; i++)
		{
			//build up the value
			char str[10];
			sprintf(str,"%02i is key",i);
			oafh_insert(&map, (ion_key_t)(&i), (unsigned char *)str);			//this is will wrap

		}

		for (i = 0; i<map.map_size; i++)
		{
			//set the position in the file
			fseek(map.file,((((i+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size)),SEEK_SET);

			status_t status;			//= ((hash_bucket_t *)(map.entry + ((((i+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size))))->status;
			int key;					//= *(int *)(((hash_bucket_t *)(map.entry + ((((i+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size))))->data );
			unsigned char value[10];	//= (ion_value_t)(((hash_bucket_t *)(map.entry + ((((i+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size))))->data + sizeof(int));

			fread(&status, SIZEOF(STATUS),1, map.file);
			fread(&key, map.super.record.key_size, 1, map.file);
			fread(value, map.super.record.value_size, 1, map.file);
			//build up expected value
			char str[10];
			sprintf(str,"%02i is key", (i+offset)%map.map_size);
			CuAssertTrue(tc, status		== IN_USE);
			CuAssertTrue(tc, key 		== (i+offset)%map.map_size);
			CuAssertStrEquals(tc, (char *)value, (char *)str);
		}
	}
	fclose(map.file);
	remove(TEST_FILE);
}
/**
@brief 		Tests a simple insert into dictionary and simple query

@details	Tests a simple insert into dictionary and simple query with the
			write_concern set to insert only
@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_simple_insert_and_query(
	CuTest		*tc
)
{
	file_hashmap_t map;								//create handler for hashmap
	int i;

	initialize_file_hash_map_std_conditions(&map);

	for (i = 0; i<map.map_size; i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		oafh_insert(&map, (ion_key_t)(&i), (ion_value_t)str);			//this is will wrap
	}

	for (i = 0; i<map.map_size; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(map.super.record.value_size);
		CuAssertTrue(tc, err_ok 	== oafh_query(&map,(ion_key_t)&i, value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertStrEquals(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}

	fclose(map.file);
	remove(TEST_FILE);

}

/**
@brief 		Tests a simple delete from dictionary

@details	Tests a simple delete from dictionary.  Builds a test collection
			and then deletes records one by one, checking that each record has
			been successfully deleted and that the rest of the map has not
			been perturbed.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_simple_delete(
	CuTest		*tc
)
{
	file_hashmap_t map;								//create handler for hashmap
	int i,j;


	initialize_file_hash_map_std_conditions(&map);

	for (i = 0; i<map.map_size; i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		oafh_insert(&map, (ion_key_t)(&i), (ion_value_t)str);			//this is will wrap
	}

	ion_value_t value;
	value = (ion_value_t)malloc(map.super.record.value_size);

	for (j = 0; j<map.map_size;j++)
	{

		//delete the record
		CuAssertTrue(tc, err_ok				== oafh_delete(&map, (ion_key_t)(&j)));
		//check to make sure that the record has been deleted
		CuAssertTrue(tc, err_item_not_found	== oafh_query(&map,(ion_key_t)(&j), value));

		//and then check to make sure that the rest of the map is undisturbed
		for (i = j+1; i<map.map_size; i++)
		{
			ion_value_t value2;
			value2 = (ion_value_t)malloc(map.super.record.value_size);
			CuAssertTrue(tc, err_ok 		== oafh_query(&map,(ion_key_t)&i, value2));
			//build up expected value
			char str[10];
			sprintf(str,"%02i is key",i);
			CuAssertStrEquals(tc, (char *)value2, str);
			if (value2 != NULL)							//must free value after query
			{
				free(value2);
			}
		}
	}
	free(value);

	fclose(map.file);
	remove(TEST_FILE);

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
test_open_address_file_hashmap_duplicate_insert_1(
	CuTest		*tc
)
{
	file_hashmap_t map;								//create handler for hashmap
	int i;

	initialize_file_hash_map_std_conditions(&map);

	//check to make sure that the write concern is set to wc_insert_unique (default)
	CuAssertTrue(tc, wc_insert_unique == map.write_concern);

	//populate the map to only half capacity to make sure there is room
	for (i = 0; i<(map.map_size/2); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertTrue(tc, err_ok  	== oafh_insert(&map,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	/** and attempt to insert values with same key, which should fail and should
	return err_duplicate_key*/
	for (i = 0; i<(map.map_size/2); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertTrue(tc, err_duplicate_key  	== oafh_insert(&map,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	fclose(map.file);
	remove(TEST_FILE);

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
test_open_address_file_hashmap_duplicate_insert_2(
	CuTest		*tc
)
{
	file_hashmap_t map;								//create handler for hashmap
	int i;

	initialize_file_hash_map_std_conditions(&map);

	//change write concern to allow up updates
	map.write_concern = wc_update;

	//check to make sure that the write concern is set to update
	CuAssertTrue(tc, wc_update == map.write_concern);

	//populate the map to only half capacity to make sure there is room
	for (i = 0; i<(map.map_size); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertTrue(tc, err_ok  	== oafh_insert(&map,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//check status of <K,V>
	for (i = 0; i<map.map_size; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(map.super.record.value_size);

		CuAssertTrue(tc, err_ok 	== oafh_query(&map,(ion_key_t)&i, value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertStrEquals(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}

	/** and attempt to insert new values with same key*/
	for (i = 0; i<(map.map_size); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is new",i);
		CuAssertTrue(tc, err_ok  	== oafh_insert(&map,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//and check updated status of <K,V>
	for (i = 0; i<map.map_size; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(map.super.record.value_size);

		CuAssertTrue(tc, err_ok 	== oafh_query(&map,(ion_key_t)&i, value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is new",i);
		CuAssertStrEquals(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}
	fclose(map.file);
	remove(TEST_FILE);
}

/**
@brief		Tests that values can be updated.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_update_1(
	CuTest		*tc
)
{
	file_hashmap_t map;								//create handler for hashmap
	int i;

	initialize_file_hash_map_std_conditions(&map);

	//change write concern to allow up updates
	map.write_concern = wc_insert_unique;

	//check to make sure that the write concern is set to wc_insert_unique (default)
	CuAssertTrue(tc, wc_insert_unique == map.write_concern);

	//populate the map to only half capacity to make sure there is room
	for (i = 0; i<(map.map_size); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertTrue(tc, err_ok  	== oafh_insert(&map,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//check status of <K,V>
	for (i = 0; i<map.map_size; i++)
	{
		ion_value_t value;;
		value = (ion_value_t)malloc(map.super.record.value_size);
		CuAssertTrue(tc, err_ok 	== oafh_query(&map,(ion_key_t)&i, value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertStrEquals(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}

	/** and update the values for the known keys */
	for (i = 0; i<(map.map_size); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is new",i);
		CuAssertTrue(tc, err_ok  	== oafh_update(&map,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//and check updated status of <K,V>
	for (i = 0; i<map.map_size; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(map.super.record.value_size);
		CuAssertTrue(tc, err_ok 	== oafh_query(&map,(ion_key_t)&i, value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is new",i);
		CuAssertStrEquals(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}
	fclose(map.file);
	remove(TEST_FILE);
}

/**
@brief		Tests that values will be ibnserted on update if they do not exist
 	 	 	in dictionary already.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_update_2(
	CuTest		*tc
)
{
	file_hashmap_t map;								//create handler for hashmap
	int i;

	initialize_file_hash_map_std_conditions(&map);

	//change write concern to allow up updates
	map.write_concern = wc_insert_unique;

	//check to make sure that the write concern is set to wc_insert_unique (default)
	CuAssertTrue(tc, wc_insert_unique == map.write_concern);

	//populate the map to only half capacity to make sure there is room
	for (i = 0; i<(map.map_size/2); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertTrue(tc, err_ok  	== oafh_insert(&map,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//check status of <K,V>
	for (i = 0; i<map.map_size/2; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(map.super.record.value_size);
		CuAssertTrue(tc, err_ok 	== oafh_query(&map,(ion_key_t)&i, value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertStrEquals(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}

	/** and update the values for the known keys */
	for (i = 0; i<(map.map_size); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is new",i);
		CuAssertTrue(tc, err_ok  	== oafh_update(&map,
													(ion_key_t)(&i),
													(ion_value_t)str));
	}

	//and check updated status of <K,V>
	for (i = 0; i<map.map_size; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(map.super.record.value_size);
		CuAssertTrue(tc, err_ok 	== oafh_query(&map,(ion_key_t)&i, value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is new",i);
		CuAssertStrEquals(tc, (char *)value, str);
		if (value != NULL)
			{
				free(value);
			}							//must free value after query
	}
	fclose(map.file);
	remove(TEST_FILE);
}

/**
@brief		Tests that values can be deleted from dictionary with single value.

@param 		tc
				CuTeest
 */
void
test_open_address_file_hashmap_delete_1(
	CuTest		*tc
)
{
	file_hashmap_t map;								//create handler for hashmap
	int i = 2;

	initialize_file_hash_map_std_conditions(&map);

	char str[10];
	sprintf(str,"%02i is key",i);
	CuAssertTrue(tc, err_ok  	== oafh_insert(&map,
									(ion_key_t)(&i),
									(ion_value_t)str));

	CuAssertTrue(tc, err_ok  	== oafh_delete(&map, (ion_key_t)(&i)));

	//Check that value is not there
	ion_value_t value;
	value = (ion_value_t)malloc(map.super.record.value_size);
	CuAssertTrue(tc, err_item_not_found
								== oafh_query(&map, (ion_key_t)(&i), value));
	if (value != NULL)
	{
		free(value);
	}

	//Check that value can not be deleted if it is not there already
	CuAssertTrue(tc, err_item_not_found
								== oafh_delete(&map, (ion_key_t)(&i)));
	fclose(map.file);
	remove(TEST_FILE);
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
test_open_address_file_hashmap_delete_2(
	CuTest		*tc
)
{
	file_hashmap_t map;								//create handler for hashmap
	int i, j;

	initialize_file_hash_map_std_conditions(&map);

	//populate the map
	for (i = 0; i<(map.map_size); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertTrue(tc, err_ok  		== oafh_insert(&map,
												(ion_key_t)(&i),
												(ion_value_t)str));
	}

	//check status of <K,V>
	for (i = 0; i<map.map_size; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(map.super.record.value_size);
		CuAssertTrue(tc, err_ok 		== oafh_query(&map,
												(ion_key_t)&i,
												value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertStrEquals(tc, (char *)value, str);
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}

	/** and update the values for the known keys */
	for (i = (map.map_size - 1); i >= 0; i--)
	{
#if DEBUG
		printf("Deleting key: %i \n",i);
#endif
		CuAssertTrue(tc, err_ok  		== oafh_delete(&map,
												(ion_key_t)(&i)));

		//Check that value is not there
		ion_value_t value;
		value = (ion_value_t)malloc(map.super.record.value_size);
		CuAssertTrue(tc, err_item_not_found
										== oafh_query(&map,
												(ion_key_t)(&i),
												value));
		if (value != NULL)
		{
			free(value);
		}

		//and check that the rest of the values are still there
		for (j = 0; j<i; j++)
		{
			ion_value_t value;
			value = (ion_value_t)malloc(map.super.record.value_size);
			CuAssertTrue(tc, err_ok 	== oafh_query(&map,
												(ion_key_t)&j,
												value));
			//build up expected value
			char str[10];
			sprintf(str,"%02i is key",j);
			CuAssertStrEquals(tc, (char *)value, str);
			if (value != NULL)							//must free value after query
			{
				free(value);
			}
		}
	}

	//Check that all the values have been deleted
	for (i = 0; i<map.map_size; i++)
	{
		ion_value_t value;
		value = (ion_value_t)malloc(map.super.record.value_size);
		CuAssertTrue(tc, err_item_not_found
										== oafh_query(&map,
												(ion_key_t)&i,
												value));
		if (value != NULL)							//must free value after query
		{
			free(value);
		}
	}
	fclose(map.file);
	remove(TEST_FILE);
}

/**
@brief		Tests that values can be inserted until capacity has been reached.

@param 		tc
				CuTest
 */
void
test_open_address_file_hashmap_capacity(
	CuTest		*tc
)
{
	file_hashmap_t map;								//create handler for hashmap
	int i;

	initialize_file_hash_map_std_conditions(&map);

	//populate the map
	for (i = 0; i<(map.map_size); i++)
	{
		//build up the value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertTrue(tc, err_ok  		== oafh_insert(&map,
												(ion_key_t)(&i),
												(ion_value_t)str));
	}

	//check status of <K,V>
	ion_value_t value;
	value = (ion_value_t)malloc(map.super.record.value_size);

	for (i = 0; i<map.map_size; i++)
	{

		CuAssertTrue(tc, err_ok 		== oafh_query(&map,
												(ion_key_t)&i,
												value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertStrEquals(tc, (char *)value, str);

	}

	//Attempt to insert a value when at max capacity
	//build up the value
	char str[10];
	i = 11;
	sprintf(str,"%02i is key",i);
	CuAssertTrue(tc, err_max_capacity 	== oafh_insert(&map,
												(ion_key_t)(&i),
												(ion_value_t)str));

	//and check to make sure that the contents has not changed
	//check status of <K,V>
	for (i = 0; i<map.map_size; i++)
	{

		CuAssertTrue(tc, err_ok 		== oafh_query(&map,
												(ion_key_t)&i,
												value));
		//build up expected value
		char str[10];
		sprintf(str,"%02i is key",i);
		CuAssertStrEquals(tc, (char *)value, str);

	}


	free(value);

	fclose(map.file);

	remove(TEST_FILE);
}



CuSuite*
open_address_file_hashmap_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_initialize);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_compute_simple_hash);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_get_location);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_find_item_location);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_simple_insert);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_simple_insert_and_query);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_simple_delete);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_duplicate_insert_1);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_duplicate_insert_2);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_update_1);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_update_2);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_delete_1);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_delete_2);
	SUITE_ADD_TEST(suite, test_open_address_file_hashmap_capacity);

	return suite;
}

void
runalltests_open_address_file_hash()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= open_address_file_hashmap_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);

	remove(TEST_FILE);
}
