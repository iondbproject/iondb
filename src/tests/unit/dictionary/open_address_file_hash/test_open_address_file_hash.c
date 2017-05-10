/**
@file
@author		Scott Ronald Fazackerley

@details	Tests base operations for open address hash map
*/

#include "test_open_address_file_hash.h"
#include "../../../../key_value/kv_system.h"

#define ION_MAX_HASH_TEST	100
#define ION_STD_MAP_SIZE	10
#define ION_TEST_FILE		"file.bin"

/**
@brief			Helper function to visualize hashmap contents

@param		  map
					The hashmap to visualize.
*/
void
check_file_map(
	ion_file_hashmap_t *map
) {
	int i;
	int bucket_size = map->super.record.key_size + map->super.record.value_size + sizeof(char);

	frewind(map->file);

	ion_hash_bucket_t *record;

	record = malloc(bucket_size);

	for (i = 0; i < map->map_size; i++) {
		DUMP(i, "%i");
		DUMP(map->map_size, "%i");

		int j;

		DUMP((int) fread(record, bucket_size, 1, map->file), "%d");
		printf("reading\n");
		fflush(stdout);

		for (j = 0; j < bucket_size; j++) {
			printf("%c ", *((char *) record) + j);
		}

		printf("\n");
		fflush(stdout);
	}

	/* and now check key positions */
}

void
initialize_file_hash_map(
	int					size,
	ion_record_info_t	*record,
	ion_file_hashmap_t	*map
) {
	map->super.compare	= dictionary_compare_signed_value;
	map->super.id		= 0;
	oafh_initialize(map, oafh_compute_simple_hash, /*dictionary_compare_signed_value,*/ map->super.key_type, record->key_size, record->value_size, size, 0);
}

void
initialize_file_hash_map_std_conditions(
	ion_file_hashmap_t *map
) {
	ion_record_info_t record;

	record.key_size		= sizeof(int);
	record.value_size	= 10;
	map->super.key_type = key_type_numeric_signed;
	initialize_file_hash_map(ION_STD_MAP_SIZE, &record, map);
}

/**
@brief	  Tests for creation and deletion of open address hash.

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_initialize(
	planck_unit_test_t *tc
) {
	/* this is required for initializing the hash map and should come from the dictionary */
	int					size;
	ion_record_info_t	record;

	record.key_size		= sizeof(int);
	record.value_size	= 10;
	size				= 10;

	ion_file_hashmap_t map;

	map.super.key_type = key_type_numeric_signed;	/* default to use int key type */

	initialize_file_hash_map(size, &record, &map);

	/* valid correct map settings */
	PLANCK_UNIT_ASSERT_TRUE(tc, map.super.record.key_size == record.key_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, map.super.record.value_size == record.value_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, map.map_size == size);
	PLANCK_UNIT_ASSERT_TRUE(tc, map.compute_hash == &oafh_compute_simple_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, map.write_concern == wc_insert_unique);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief		Tests the computation of a simple hash value

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_compute_simple_hash(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;		/* create handler for hashmap */
	int					i;

	initialize_file_hash_map_std_conditions(&map);

	for (i = 0; i < ION_MAX_HASH_TEST; i++) {
		PLANCK_UNIT_ASSERT_TRUE(tc, (i % map.map_size) == oafh_compute_simple_hash(&map, ((int *) &i), sizeof(i)));
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief	  Test locating an element in the hashmap and returns the theoretical
			location of the item.

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_get_location(
	planck_unit_test_t *tc
) {
	int i;

	for (i = 0; i < ION_MAX_HASH_TEST; i++) {
		PLANCK_UNIT_ASSERT_TRUE(tc, (i % ION_STD_MAP_SIZE) == oafh_get_location((ion_hash_t) i, ION_STD_MAP_SIZE));
	}
}

/**
@brief		Test locating an element in the hashmap and returns the physical
			address of the item.

@details	Test locating an element in the hashmap and returns the physical
			address of the item.  May probe multiple locations depending of
			current occupancy of hashmap

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_find_item_location(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;		/* create handler for hashmap */
	int					i;
	int					offset;

	initialize_file_hash_map_std_conditions(&map);

	/*Manually populate records */
	ion_record_info_t record = map.super.record;

	char *item;

	/* manually map out item stucture */
	item = malloc(sizeof(char) * (record.key_size + record.value_size + sizeof(char)));

	/* manually populate array */
	ion_hash_bucket_t *item_ptr = (ion_hash_bucket_t *) item;

	/* FILE *file_ptr			   = fopen("test.bin","w+"); */
	/* map.file					 = file_ptr; */

	int bucket_size = sizeof(char) + record.key_size + record.value_size;

	/* rewind */
	frewind(map.file);

	for (offset = 0; offset < map.map_size; offset++) {
		/* apply continual offsets */
#if ION_DEBUG
		printf("entry loc: %p %p \n", map.entry, pos_ptr);
#endif
		/* pos_ptr			  = (map.entry */
		/*							+ (offset*bucket_size)%(map.map_size*bucket_size)); */

		/* printf("writing to %i\n",(offset*bucket_size)%(map.map_size*bucket_size)); */

		fseek(map.file, (offset * bucket_size) % (map.map_size * bucket_size), SEEK_SET);

		for (i = 0; i < map.map_size; i++) {
			item_ptr->status = ION_IN_USE;
			memcpy(item_ptr->data, (int *) &i, sizeof(int));

			char str[10];

			/* build up the value */
			sprintf(str, "%02i is key", i);
			/* and copy it directly into the slot */
			memcpy((item_ptr->data + sizeof(int)), str, 10);

			/* memcpy(pos_ptr, item_ptr, bucket_size); */

			fwrite(item_ptr, bucket_size, 1, map.file);
			/* printf("Moving to position %i\n", ((((i+1+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size))); */
			/* pos_ptr = map.entry + ((((i+1+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size)); */
			fseek(map.file, ((((i + 1 + offset) % map.map_size) * bucket_size) % (map.map_size * bucket_size)), SEEK_SET);
			/* printf("current file pos: %i\n",(int)	ftell(map.file)); */
		}

		/* and now check key positions */
		for (i = 0; i < map.map_size; i++) {
			int location;

			PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_find_item_loc(&map, (&i), &location));
			/* printf("location %i\n",location); */
			PLANCK_UNIT_ASSERT_TRUE(tc, (i + offset) % map.map_size == location);
		}
	}

	free(item);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief	  Tests a simple insert into map and reads results directly from map

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_simple_insert(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;		/* create handler for hashmap */
	int					i;
	int					offset = 0;
	ion_status_t		status;

	initialize_file_hash_map_std_conditions(&map);

	/* check_file_map(&map); */

	/* Manually populate records */
	ion_record_info_t record	= map.super.record;

	int bucket_size				= sizeof(char) + record.key_size + record.value_size;

	/* rewind */
	frewind(map.file);

	for (offset = 0; offset < map.map_size; offset++) {
		for (i = 0; i < map.map_size; i++) {
			/* build up the value */
			char str[10];

			sprintf(str, "%02i is key", i);
			status = oafh_insert(&map, (&i), (ion_byte_t *) str);	/* this is will wrap */

			if ((0 == offset) || (wc_duplicate == map.write_concern)) {
				PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
				PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
			}
		}

		for (i = 0; i < map.map_size; i++) {
			/* set the position in the file */
			fseek(map.file, ((((i + offset) % map.map_size) * bucket_size) % (map.map_size * bucket_size)), SEEK_SET);

			ion_record_status_t record_status;	/* = ((ion_hash_bucket_t *)(map.entry + ((((i+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size))))->status; */
			int					key;	/* = *(int *)(((ion_hash_bucket_t *)(map.entry + ((((i+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size))))->data ); */
			ion_byte_t			value[10];		/* = (((ion_hash_bucket_t *)(map.entry + ((((i+offset)%map.map_size)*bucket_size )%(map.map_size*bucket_size))))->data + sizeof(int)); */

			fread(&record_status, SIZEOF(STATUS), 1, map.file);
			fread(&key, map.super.record.key_size, 1, map.file);
			fread(value, map.super.record.value_size, 1, map.file);

			/* build up expected value */
			char str[10];

			sprintf(str, "%02i is key", (i + offset) % map.map_size);
			PLANCK_UNIT_ASSERT_TRUE(tc, record_status == ION_IN_USE);
			PLANCK_UNIT_ASSERT_TRUE(tc, key == (i + offset) % map.map_size);
			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, (char *) str);
		}
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief	  Tests a simple insert into dictionary and simple query

@details	Tests a simple insert into dictionary and simple query with the
			write_concern set to insert only
@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_simple_insert_and_query(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;							/* create handler for hashmap */
	int					i;
	ion_status_t		status;

	initialize_file_hash_map_std_conditions(&map);

	for (i = 0; i < map.map_size; i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		oafh_insert(&map, (&i), str);	/* this is will wrap */
	}

	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= malloc(map.super.record.value_size);
		status	= oafh_get(&map, &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is key", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);

		if (value != NULL) {
			/* must free value after query */
			free(value);
		}
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief	  Tests a simple delete from dictionary

@details	Tests a simple delete from dictionary.  Builds a test dictionary
			and then deletes records one by one, checking that each record has
			been successfully deleted and that the rest of the map has not
			been perturbed.

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_simple_delete(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;							/* create handler for hashmap */
	int					i, j;
	ion_status_t		status;

	initialize_file_hash_map_std_conditions(&map);

	for (i = 0; i < map.map_size; i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oafh_insert(&map, (&i), str);	/* this is will wrap */
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	ion_value_t value;

	value = malloc(map.super.record.value_size);

	for (j = 0; j < map.map_size; j++) {
		/* delete the record */
		status = oafh_delete(&map, (&j));
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
		/* check to make sure that the record has been deleted */
		status = oafh_get(&map, (&j), value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

		/* and then check to make sure that the rest of the map is undisturbed */
		for (i = j + 1; i < map.map_size; i++) {
			ion_value_t value2;

			value2	= malloc(map.super.record.value_size);
			status	= oafh_get(&map, &i, value2);
			PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
			PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

			/* build up expected value */
			char str[10];

			sprintf(str, "%02i is key", i);
			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value2, str);

			if (value2 != NULL) {
				/* must free value after query */
				free(value2);
			}
		}
	}

	free(value);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief		Tests that duplicate values are not allowed with write_concern
			is set for wc_insert_unique.

@details	Tests that duplicate values are not allowed with write_concern
			is set for wc_insert_unique.   Will generate error.

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_duplicate_insert_1(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;							/* create handler for hashmap */
	int					i;
	ion_status_t		status;

	initialize_file_hash_map_std_conditions(&map);

	/* check to make sure that the write concern is set to wc_insert_unique (default) */
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_insert_unique == map.write_concern);

	/* populate the map to only half capacity to make sure there is room */
	for (i = 0; i < (map.map_size / 2); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oafh_insert(&map, (&i), str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/*and attempt to insert values with same key, which should fail and should
	return err_duplicate_key*/
	for (i = 0; i < (map.map_size / 2); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oafh_insert(&map, (&i), str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_duplicate_key == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief		Tests that value updates are allowed with write_concern
			is set for wc_update.

@details	Tests that duplicate values are not allowed with write_concern
			is set for wc_update but will update the value.

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_duplicate_insert_2(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;							/* create handler for hashmap */
	int					i;
	ion_status_t		status;

	initialize_file_hash_map_std_conditions(&map);

	/* change write concern to allow up updates */
	map.write_concern = wc_update;

	/* check to make sure that the write concern is set to update */
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_update == map.write_concern);

	/* populate the map to only half capacity to make sure there is room */
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oafh_insert(&map, (&i), str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* check status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= malloc(map.super.record.value_size);

		status	= oafh_get(&map, &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is key", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);

		if (value != NULL) {
			/* must free value after query */
			free(value);
		}
	}

	/*and attempt to insert new values with same key*/
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is new", i);
		status = oafh_insert(&map, (&i), str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* and check updated status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= malloc(map.super.record.value_size);

		status	= oafh_get(&map, &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is new", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);

		if (value != NULL) {
			/* must free value after query */
			free(value);
		}
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief		Tests that values can be updated.

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_update_1(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;							/* create handler for hashmap */
	int					i;
	ion_status_t		status;

	initialize_file_hash_map_std_conditions(&map);

	/* change write concern to allow up updates */
	map.write_concern = wc_insert_unique;

	/* check to make sure that the write concern is set to wc_insert_unique (default) */
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_insert_unique == map.write_concern);

	/* populate the map to only half capacity to make sure there is room */
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oafh_insert(&map, (&i), str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* check status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		;

		value	= malloc(map.super.record.value_size);
		status	= oafh_get(&map, &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is key", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);

		if (value != NULL) {
			/* must free value after query */
			free(value);
		}
	}

	/*and update the values for the known keys */
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is new", i);
		status = oafh_update(&map, (&i), str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* and check updated status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= malloc(map.super.record.value_size);
		status	= oafh_get(&map, &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is new", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);

		if (value != NULL) {
			/* must free value after query */
			free(value);
		}
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief		Tests that values will be ibnserted on update if they do not exist
			in dictionary already.

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_update_2(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;							/* create handler for hashmap */
	int					i;
	ion_status_t		status;

	initialize_file_hash_map_std_conditions(&map);

	/* change write concern to allow up updates */
	map.write_concern = wc_insert_unique;

	/* check to make sure that the write concern is set to wc_insert_unique (default) */
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_insert_unique == map.write_concern);

	/* populate the map to only half capacity to make sure there is room */
	for (i = 0; i < (map.map_size / 2); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oafh_insert(&map, (&i), str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* check status of <K,V> */
	for (i = 0; i < map.map_size / 2; i++) {
		ion_value_t value;

		value	= malloc(map.super.record.value_size);
		status	= oafh_get(&map, &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is key", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);

		if (value != NULL) {
			/* must free value after query */
			free(value);
		}
	}

	/*and update the values for the known keys */
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is new", i);
		status = oafh_update(&map, (&i), str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* and check updated status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= malloc(map.super.record.value_size);
		status	= oafh_get(&map, &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is new", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);

		if (value != NULL) {
			free(value);
		}	/* must free value after query */
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief		Tests that values can be deleted from dictionary with single value.

@param	  tc
				CuTeest
*/
void
test_open_address_file_hashmap_delete_1(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;							/* create handler for hashmap */
	int					i = 2;
	ion_status_t		status;

	initialize_file_hash_map_std_conditions(&map);

	char str[10];

	sprintf(str, "%02i is key", i);
	status = oafh_insert(&map, (&i), str);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	status = oafh_delete(&map, (&i));
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	/* Check that value is not there */
	ion_value_t value;

	value	= malloc(map.super.record.value_size);
	status	= oafh_get(&map, (&i), value);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

	if (value != NULL) {
		free(value);
	}

	/* Check that value can not be deleted if it is not there already */
	status = oafh_delete(&map, (&i));
	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief		Tests that values can be deleted from dictionary with multiple
			values.

@details	Tests that values can be deleted from dictionary with multiple
			values.  Dictionary will have multiple values and only a single
			value will be deleted.  Checks to ensure that other values are
			undisturbed.

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_delete_2(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;							/* create handler for hashmap */
	int					i, j;
	ion_status_t		status;

	initialize_file_hash_map_std_conditions(&map);

	/* populate the map */
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oafh_insert(&map, (&i), str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* check status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= malloc(map.super.record.value_size);
		status	= oafh_get(&map, &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is key", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);

		if (value != NULL) {
			/* must free value after query */
			free(value);
		}
	}

	/*and update the values for the known keys */
	for (i = (map.map_size - 1); i >= 0; i--) {
#if ION_DEBUG
		printf("Deleting key: %i \n", i);
#endif
		status = oafh_delete(&map, (&i));
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* Check that value is not there */
		ion_value_t value;

		value	= malloc(map.super.record.value_size);
		status	= oafh_get(&map, (&i), value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

		if (value != NULL) {
			free(value);
		}

		/* and check that the rest of the values are still there */
		for (j = 0; j < i; j++) {
			ion_value_t value;

			value	= malloc(map.super.record.value_size);
			status	= oafh_get(&map, &j, value);
			PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
			PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

			/* build up expected value */
			char str[10];

			sprintf(str, "%02i is key", j);
			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);

			if (value != NULL) {
				/* must free value after query */
				free(value);
			}
		}
	}

	/* Check that all the values have been deleted */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= malloc(map.super.record.value_size);
		status	= oafh_get(&map, &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

		if (value != NULL) {
			/* must free value after query */
			free(value);
		}
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

/**
@brief		Tests that values can be inserted until capacity has been reached.

@param	  tc
				Test case.
*/
void
test_open_address_file_hashmap_capacity(
	planck_unit_test_t *tc
) {
	ion_file_hashmap_t	map;							/* create handler for hashmap */
	int					i;
	ion_status_t		status;

	initialize_file_hash_map_std_conditions(&map);

	/* populate the map */
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oafh_insert(&map, (&i), str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* check status of <K,V> */
	ion_value_t value;

	value = malloc(map.super.record.value_size);

	for (i = 0; i < map.map_size; i++) {
		status = oafh_get(&map, &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is key", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);
	}

	/* Attempt to insert a value when at max capacity */
	/* build up the value */
	char str[10];

	i		= 11;
	sprintf(str, "%02i is key", i);
	status	= oafh_insert(&map, (&i), str);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_max_capacity == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

	/* and check to make sure that the contents has not changed */
	/* check status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		status = oafh_get(&map, &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is key", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);
	}

	free(value);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oafh_destroy(&map));
}

planck_unit_suite_t *
open_address_file_hashmap_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_initialize);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_compute_simple_hash);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_get_location);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_find_item_location);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_simple_insert);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_simple_insert_and_query);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_simple_delete);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_duplicate_insert_1);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_duplicate_insert_2);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_update_1);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_update_2);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_delete_1);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_delete_2);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_open_address_file_hashmap_capacity);

	return suite;
}

void
runalltests_open_address_file_hash(
) {
	/* CuString	*output	= CuStringNew(); */
	planck_unit_suite_t *suite = open_address_file_hashmap_getsuite();

	planck_unit_run_suite(suite);
	/* CuSuiteSummary(suite, output); */
	/* CuSuiteDetails(suite, output); */
	/* printf("%s\n", output->buffer); */

	planck_unit_destroy_suite(suite);
	/* CuSuiteDelete(suite); */
	/* CuStringDelete(output); */

	fremove("0.oaf");
}
