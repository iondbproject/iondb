/**
@file
@author		Scott Ronald Fazackerley

@details	Tests base operations for open address hash map
 */

#include "test_open_address_hash.h"
#include "../../../../kv_system.h"

#define MAX_HASH_TEST	100
#define STD_MAP_SIZE	10

/**
@brief			Helper function to visualize hashmap contents

@param		  map
					The hashmap to visualize.
 */
void
check_map(
	hashmap_t *map
) {
	int i;
	int bucket_size = map->super.record.key_size + map->super.record.value_size + sizeof(char);

	for (i = 0; i < map->map_size; i++) {
		int j;

		for (j = 0; j < bucket_size; j++) {
			printf("%X ", *(char *) (map->entry + i * bucket_size + j));
		}

		printf("\n");
	}

	/* and now check key positions */
}

void
initialize_hash_map(
	int				size,
	record_info_t	*record,
	hashmap_t		*map
) {
	oah_initialize(map, oah_compute_simple_hash, /*dictionary_compare_signed_value,*/ map->super.key_type, record->key_size, record->value_size, size);
	map->super.compare = dictionary_compare_signed_value;
}

void
initialize_hash_map_std_conditions(
	hashmap_t *map
) {
	record_info_t record;

	record.key_size		= 4;
	record.value_size	= 10;
	map->super.key_type = key_type_numeric_signed;
	initialize_hash_map(STD_MAP_SIZE, &record, map);
}

/**
@brief	  Tests for creation and deletion of open address hash.

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_initialize(
	planck_unit_test_t *tc
) {
	/* this is required for initializing the hash map and should come from the dictionary */
	int				size;
	record_info_t	record;

	record.key_size		= 4;
	record.value_size	= 10;
	size				= 10;

	hashmap_t map;

	map.super.key_type = key_type_numeric_signed;	/* default to use int key type */

	initialize_hash_map(size, &record, &map);

	/* valid correct map settings */
	PLANCK_UNIT_ASSERT_TRUE(tc, map.super.record.key_size == record.key_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, map.super.record.value_size == record.value_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, map.map_size == size);
	PLANCK_UNIT_ASSERT_TRUE(tc, map.compute_hash == &oah_compute_simple_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, map.write_concern == wc_insert_unique);
}

/**
@brief		Tests the computation of a simple hash value

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_compute_simple_hash(
	planck_unit_test_t *tc
) {
	hashmap_t	map;		/* create handler for hashmap */
	int			i;

	initialize_hash_map_std_conditions(&map);

	for (i = 0; i < MAX_HASH_TEST; i++) {
		PLANCK_UNIT_ASSERT_TRUE(tc, (i % map.map_size) == oah_compute_simple_hash(&map, (ion_key_t) ((int *) &i), sizeof(i)));
	}
}

/**
@brief	  Test locating an element in the hashmap and returns the theoretical
			location of the item.

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_get_location(
	planck_unit_test_t *tc
) {
	int i;

	for (i = 0; i < MAX_HASH_TEST; i++) {
		PLANCK_UNIT_ASSERT_TRUE(tc, (i % STD_MAP_SIZE) == oah_get_location((hash_t) i, STD_MAP_SIZE));
	}
}

/**
@brief		Test locating an element in the hashmap and returns the physical
			address of the item.

@details	Test locating an element in the hashmap and returns the physical
			address of the item.  May probe multiple locations depending of
			current occupancy of hashmap

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_find_item_location(
	planck_unit_test_t *tc
) {
	hashmap_t	map;		/* create handler for hashmap */
	int			i;
	int			offset;

	initialize_hash_map_std_conditions(&map);

	/*Manually populate records */
	record_info_t record = map.super.record;

	char *item;

	/* manually map out item stucture */
	item = (char *) malloc(sizeof(char) * (record.key_size + record.value_size + sizeof(char)));

	/* manually populate array */
	hash_bucket_t	*item_ptr	= (hash_bucket_t *) item;
	char			*pos_ptr	= map.entry;

	/* Bucket size includes flags, data, value */
	int bucket_size				= sizeof(char) + record.key_size + record.value_size;

	for (offset = 0; offset < map.map_size; offset++) {
		/* apply continual offsets to traverse map */
#if DEBUG
		printf("entry loc: %p %p \n", map.entry, pos_ptr);
#endif
		pos_ptr = (map.entry + (offset * bucket_size) % (map.map_size * bucket_size));

		for (i = 0; i < map.map_size; i++) {
			item_ptr->status = IN_USE;
			/* Ensure to use key_size */
			memcpy(item_ptr->data, (ion_key_t) &i, record.key_size);

			/* build up the value */
			char str[10];

			sprintf(str, "%02i is key", i);
			/* Copy it directly into the slot */
			memcpy((item_ptr->data + record.key_size), str, 10);
			memcpy(pos_ptr, item_ptr, bucket_size);
			pos_ptr = map.entry + ((((i + 1 + offset) % map.map_size) * bucket_size) % (map.map_size * bucket_size));
		}

		/* and now check key positions */
		for (i = 0; i < map.map_size; i++) {
			int location;

			PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == oah_find_item_loc(&map, (ion_key_t) (&i), &location));
			PLANCK_UNIT_ASSERT_TRUE(tc, (i + offset) % map.map_size == location);
		}
	}
}

/**
@brief	  Tests a simple insert into map and reads results directly from map.

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_simple_insert(
	planck_unit_test_t *tc
) {
	/* create handler for hashmap */
	hashmap_t	map;
	int			i;
	int			offset;

	initialize_hash_map_std_conditions(&map);

	/*Manually populate records */
	record_info_t record = map.super.record;

	/* manually populate array */
#if DEBUG

	char *pos_ptr = map.entry;

#endif

	int bucket_size = sizeof(char) + record.key_size + record.value_size;

	for (offset = 0; offset < map.map_size; offset++) {
		/* apply continual offsets */
#if DEBUG
		printf("entry loc: %p %p \n", map.entry, pos_ptr);
		pos_ptr = (map.entry + (offset * bucket_size) % (map.map_size * bucket_size));
#endif

		for (i = 0; i < map.map_size; i++) {
			/* build up the value */
			ion_byte_t str[10];

			sprintf((char *) str, "%02i is key", i);

			ion_status_t status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);/* this is will wrap */

			/* FIXME: Why are we inserting inside two loops?!? */
			if ((offset == 0) || (wc_duplicate == map.write_concern)) {
				PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
				PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
			}
		}

		for (i = 0; i < map.map_size; i++) {
			ion_record_status_t status	= ((hash_bucket_t *) (map.entry + ((((i + offset) % map.map_size) * bucket_size) % (map.map_size * bucket_size))))->status;
			ion_key_t			key		= (ion_key_t) (((hash_bucket_t *) (map.entry + ((((i + offset) % map.map_size) * bucket_size) % (map.map_size * bucket_size))))->data);
			ion_value_t			value	= (ion_value_t) (((hash_bucket_t *) (map.entry + ((((i + offset) % map.map_size) * bucket_size) % (map.map_size * bucket_size))))->data + record.key_size);

			/* build up expected value */
			ion_byte_t str[10];

			sprintf((char *) str, "%02i is key", (i + offset) % map.map_size);
			PLANCK_UNIT_ASSERT_TRUE(tc, status == IN_USE);
			/* check to ensure key is pointing at correct location */
			PLANCK_UNIT_ASSERT_TRUE(tc, *(int *) key == (i + offset) % map.map_size);
			/* While str and value are unsigned, equality test is signed, so casting required */
			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) str, (char *) value);
		}
	}
}

/**
@brief	  Tests a simple insert into dictionary and simple query.

@details	Tests a simple insert into dictionary and simple query with the
			write_concern set to insert only
@param	  tc
				CuTest
 */
void
test_open_address_hashmap_simple_insert_and_query(
	planck_unit_test_t *tc
) {
	/* create handler for hashmap */
	hashmap_t		map;
	int				i;
	ion_status_t	status;

	initialize_hash_map_std_conditions(&map);

	for (i = 0; i < map.map_size; i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		/* this is will wrap the map */
		status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	ion_value_t value;

	value = (ion_value_t) malloc(10);

	for (i = 0; i < map.map_size; i++) {
		status = oah_query(&map, (ion_key_t) &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is key", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);
	}

	/* must free value after query */
	if (value != NULL) {
		free(value);
	}
}

/**
@brief	  Tests a simple delete from dictionary

@details	Tests a simple delete from dictionary.  Builds a test collection
			and then deletes records one by one, checking that each record has
			been successfully deleted and that the rest of the map has not
			been perturbed.

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_simple_delete(
	planck_unit_test_t *tc
) {
	/* create handler for hashmap */
	hashmap_t		map;
	int				i, j;
	ion_status_t	status;

	initialize_hash_map_std_conditions(&map);

	ion_value_t value;

	value = (ion_value_t) malloc(10);

	for (i = 0; i < map.map_size; i++) {
		/* build up the value */
		ion_byte_t str[10];

		sprintf((char *) str, "%02i is key", i);
		/* this is will wrap the map*/
		status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		status = oah_query(&map, (ion_key_t) (&i), value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	i		= 0;
	status	= oah_query(&map, (ion_key_t) (&i), value);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	for (j = 0; j < map.map_size; j++) {
		/* delete the record */
		status = oah_delete(&map, (ion_key_t) (&j));
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* check to make sure that the record has been deleted */
		status = oah_query(&map, (ion_key_t) (&j), value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

		/* and then check to make sure that the rest of the map is undisturbed */
		for (i = j + 1; i < map.map_size; i++) {
/*
			ion_value_t value;
*/
			/*value	 = (ion_value_t)malloc(10);*/

			status = oah_query(&map, (ion_key_t) &i, value);
			PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
			PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

			/* build up expected value */
			char str[10];

			sprintf(str, "%02i is key", i);
			PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);

			/*if (value != NULL) {
				free(value);
			}*/
		}
	}

	/* must free value after query */
	if (value != NULL) {
		free(value);
	}
}

/**
@brief		Tests that duplicate values are not allowed with write_concern
			is set for wc_insert_unique.

@details	Tests that duplicate values are not allowed with write_concern
			is set for wc_insert_unique.   Will generate error.

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_duplicate_insert_1(
	planck_unit_test_t *tc
) {
	hashmap_t		map;						/* create handler for hashmap */
	int				i;
	ion_status_t	status;

	initialize_hash_map_std_conditions(&map);

	/* check to make sure that the write concern is set to wc_insert_unique (default) */
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_insert_unique == map.write_concern);

	/* populate the map to only half capacity to make sure there is room */
	for (i = 0; i < (map.map_size / 2); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/*and attempt to insert values with same key, which should fail and should
	return err_duplicate_key*/
	for (i = 0; i < (map.map_size / 2); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_duplicate_key == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);
	}
}

/**
@brief		Tests that value updates are allowed with write_concern
			is set for wc_update.

@details	Tests that duplicate values are not allowed with write_concern
			is set for wc_update but will update the value.

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_duplicate_insert_2(
	planck_unit_test_t *tc
) {
	hashmap_t		map;						/* create handler for hashmap */
	int				i;
	ion_status_t	status;

	initialize_hash_map_std_conditions(&map);

	/* change write concern to allow up updates */
	map.write_concern = wc_update;

	/* check to make sure that the write concern is set to wc_insert_unique (default) */
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_update == map.write_concern);

	/* populate the map to only half capacity to make sure there is room */
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* check status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= (ion_value_t) malloc(10);
		status	= oah_query(&map, (ion_key_t) &i, value);
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

	/*and attempt to insert values with same key, which should fail and should
	return err_duplicate_key*/
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is new", i);
		status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* and check updated status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= (ion_value_t) malloc(10);
		status	= oah_query(&map, (ion_key_t) &i, value);
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
}

/**
@brief		Tests that values can be updated.

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_update_1(
	planck_unit_test_t *tc
) {
	hashmap_t		map;						/* create handler for hashmap */
	int				i;
	ion_status_t	status;

	initialize_hash_map_std_conditions(&map);

	/* change write concern to allow up updates */
	map.write_concern = wc_insert_unique;

	/* check to make sure that the write concern is set to wc_insert_unique (default) */
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_insert_unique == map.write_concern);

	/* populate the map to only half capacity to make sure there is room */
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* check status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= (ion_value_t) malloc(10);
		status	= oah_query(&map, (ion_key_t) &i, value);
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
		status = oah_update(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* and check updated status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= (ion_value_t) malloc(10);
		status	= oah_query(&map, (ion_key_t) &i, value);
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
}

/**
@brief		Tests that values will be inserted on update if they do not exist
			in dictionary already.

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_update_2(
	planck_unit_test_t *tc
) {
	hashmap_t		map;						/* create handler for hashmap */
	int				i;
	ion_status_t	status;

	initialize_hash_map_std_conditions(&map);

	/* change write concern to allow up updates */
	map.write_concern = wc_insert_unique;

	/* check to make sure that the write concern is set to wc_insert_unique (default) */
	PLANCK_UNIT_ASSERT_TRUE(tc, wc_insert_unique == map.write_concern);

	/* populate the map to only half capacity to make sure there is room */
	for (i = 0; i < (map.map_size / 2); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* check status of <K,V> */
	for (i = 0; i < map.map_size / 2; i++) {
		ion_value_t value;

		value	= (ion_value_t) malloc(10);
		status	= oah_query(&map, (ion_key_t) &i, value);
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
		status = oah_update(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* and check updated status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= (ion_value_t) malloc(10);
		status	= oah_query(&map, (ion_key_t) &i, value);
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
}

/**
@brief		Tests that values can be deleted from dictionary with single value.

@param	  tc
				CuTeest
 */
void
test_open_address_hashmap_delete_1(
	planck_unit_test_t *tc
) {
	hashmap_t		map;						/* create handler for hashmap */
	int				i = 2;
	ion_status_t	status;

	initialize_hash_map_std_conditions(&map);

	char str[10];

	sprintf(str, "%02i is key", i);
	status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	status = oah_delete(&map, (ion_key_t) (&i));
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	/* Check that value is not there */
	ion_value_t value;

	value	= (ion_value_t) malloc(10);
	status	= oah_query(&map, (ion_key_t) (&i), value);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

	if (value != NULL) {
		free(value);
	}

	/* Check that value can not be deleted if it is not there already */
	status = oah_delete(&map, (ion_key_t) (&i));
	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);
}

/**
@brief		Tests that values can be deleted from dictionary with multiple
			values.

@details	Tests that values can be deleted from dictionary with multiple
			values.  Dictionary will have multiple values and only a single
			value will be deleted.  Checks to ensure that other values are
			undisturbed.

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_delete_2(
	planck_unit_test_t *tc
) {
	hashmap_t		map;						/* create handler for hashmap */
	int				i, j;
	ion_status_t	status;

	initialize_hash_map_std_conditions(&map);

	/* populate the map */
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* check status of <K,V> */
	for (i = 0; i < map.map_size; i++) {
		ion_value_t value;

		value	= (ion_value_t) malloc(10);
		status	= oah_query(&map, (ion_key_t) &i, value);
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
#if DEBUG
		printf("Deleting key: %i \n", i);
#endif
		status = oah_delete(&map, (ion_key_t) (&i));
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* Check that value is not there */
		ion_value_t value;

		value	= (ion_value_t) malloc(10);
		status	= oah_query(&map, (ion_key_t) (&i), value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

		if (value != NULL) {
			free(value);
		}

		/* and check that the rest of the values are still there */
		for (j = 0; j < i; j++) {
			ion_value_t value;

			value	= (ion_value_t) malloc(10);
			status	= oah_query(&map, (ion_key_t) &j, value);
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

		value	= (ion_value_t) malloc(10);
		status	= oah_query(&map, (ion_key_t) &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

		if (value != NULL) {
			/* must free value after query */
			free(value);
		}
	}
}

/**
@brief		Tests that values can be inserted until capacity has been reached.

@param	  tc
				CuTest
 */
void
test_open_address_hashmap_capacity(
	planck_unit_test_t *tc
) {
	hashmap_t		map;						/* create handler for hashmap */
	int				i;
	ion_status_t	status;

	initialize_hash_map_std_conditions(&map);

	/* populate the map */
	for (i = 0; i < (map.map_size); i++) {
		/* build up the value */
		char str[10];

		sprintf(str, "%02i is key", i);
		status = oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	/* check status of <K,V> */
	ion_value_t value;

	value = (ion_value_t) malloc(10);

	for (i = 0; i < map.map_size; i++) {
		status = oah_query(&map, (ion_key_t) &i, value);
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
	status	= oah_insert(&map, (ion_key_t) (&i), (ion_value_t) str);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_max_capacity == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

	/* and check to make sure that the contents has not changed */
	/* check status of <K,V> */
	value = (ion_value_t) malloc(10);

	for (i = 0; i < map.map_size; i++) {
		status = oah_query(&map, (ion_key_t) &i, value);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		/* build up expected value */
		char str[10];

		sprintf(str, "%02i is key", i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, str);
	}

	free(value);
}

planck_unit_suite_t *
open_address_hashmap_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	planck_unit_add_to_suite(suite, test_open_address_hashmap_initialize);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_compute_simple_hash);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_get_location);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_find_item_location);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_simple_insert);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_simple_insert_and_query);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_simple_delete);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_duplicate_insert_1);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_duplicate_insert_2);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_update_1);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_update_2);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_delete_1);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_delete_2);
	planck_unit_add_to_suite(suite, test_open_address_hashmap_capacity);

	return suite;
}

void
runalltests_open_address_hash(
) {
	/* CuString	*output	= CuStringNew(); */
	planck_unit_suite_t *suite = open_address_hashmap_getsuite();

	planck_unit_run_suite(suite);
	/* CuSuiteSummary(suite, output); */
	/* CuSuiteDetails(suite, output); */
	/* printf("%s\n", output->buffer); */

	planck_unit_destroy_suite(suite);
	/* CuSuiteDelete(suite); */
	/* CuStringDelete(output); */
}
