/******************************************************************************/
/**
@file
@author		Spencer MacBeth
@brief		Implementation unit tests for the linear hash.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
*/
/******************************************************************************/

#include "test_linear_hash.h"

/**
@brief		Initializes a test flatfile instance and does a few sanity checks.
*/
void
test_linear_hash_create(
	planck_unit_test_t		*tc,
	linear_hash_table_t		*linear_hash,
	ion_key_type_t			key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size
) {
	/* TODO create and desetroy a linear_hash in every file */

	int			initial_size		= 5;
	int			split_threshold		= 85;
	int			records_per_bucket	= 4;
	ion_err_t	err					= linear_hash_init(1, dictionary_size, key_type, key_size, value_size, initial_size, split_threshold, records_per_bucket, linear_hash);

	linear_hash->super.compare = dictionary_compare_signed_value;

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != linear_hash->database);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != linear_hash->state);
	PLANCK_UNIT_ASSERT_TRUE(tc, initial_size == linear_hash->num_buckets);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == linear_hash->num_records);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == err);
}

/**
@brief		Destroys a flatfile instance and checks to ensure everything is cleaned up.
*/
void
test_linear_hash_destroy(
	planck_unit_test_t	*tc,
	linear_hash_table_t *linear_hash
) {
	ion_err_t err = linear_hash_destroy(linear_hash);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == linear_hash->database);

	free(linear_hash);
}

/**
@brief		Sets up the default test instance we're going to use.
*/
void
test_linear_hash_setup(
	planck_unit_test_t	*tc,
	linear_hash_table_t *linear_hash
) {
	test_linear_hash_create(tc, linear_hash, key_type_numeric_signed, sizeof(int), sizeof(int), 4);
}

/**
@brief		Tears down the default test instance we're going to use.
*/
void
test_linear_hash_takedown(
	planck_unit_test_t	*tc,
	linear_hash_table_t *linear_hash
) {
	test_linear_hash_destroy(tc, linear_hash);
}

void
test_linear_hash_get(
	planck_unit_test_t	*tc,
	ion_key_t			key,
	ion_err_t			expected_status,
	ion_result_count_t	expected_count,
	ion_value_t			expected_value,
	linear_hash_table_t *linear_hash
) {
	ion_value_t defaultval = alloca(linear_hash->super.record.value_size);

	memset(defaultval, 0x76, linear_hash->super.record.value_size);

	ion_value_t retval = alloca(linear_hash->super.record.value_size);

	memcpy(retval, defaultval, linear_hash->super.record.value_size);

	ion_status_t status = linear_hash_get(key, retval, linear_hash);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (err_ok == status.error) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, memcmp(expected_value, retval, linear_hash->super.record.value_size));
	}
	else {
		/* Here, we check to see that the passed in space to write the value remains unchanged, if we have an error condition. */
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, memcmp(defaultval, retval, linear_hash->super.record.value_size));
	}


}

/**
@brief		Inserts into the flat file and optionally checks if the insert was OK
			by reading the data file. Don't turn on check_result unless you expect
			the insert to fully pass.
*/
void
test_linear_hash_insert(
	planck_unit_test_t	*tc,
	ion_key_t			key,
	ion_value_t			value,
	ion_err_t			expected_status,
	ion_result_count_t	expected_count,
	ion_boolean_t		check_result,
	linear_hash_table_t *linear_hash
) {
	int bucket_idx = insert_hash_to_bucket(key, linear_hash);

	if (bucket_idx < linear_hash->next_split) {
		bucket_idx = hash_to_bucket(key, linear_hash);
	}

	ion_status_t status = linear_hash_insert(key, value, bucket_idx, linear_hash);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (check_result) {
		test_linear_hash_get(tc, key, err_ok, 1, value, linear_hash);
	}
}

/**
@brief		Updates in the flatfile and asserts everything went as expected.
*/
void
test_linear_hash_update(
	planck_unit_test_t	*tc,
	ion_key_t			key,
	ion_value_t			value,
	ion_err_t			expected_status,
	ion_result_count_t	expected_count,
	linear_hash_table_t *linear_hash
) {
	ion_status_t status = linear_hash_update(key, value, linear_hash);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (err_ok == expected_status) {
		test_linear_hash_get(tc, key, expected_status, expected_count, value, linear_hash);
	}
}

/**
@brief		Updates in the flatfile and asserts everything went as expected.
*/
void
test_linear_hash_delete(
	planck_unit_test_t	*tc,
	ion_key_t			key,
	ion_err_t			expected_status,
	ion_result_count_t	expected_count,
	linear_hash_table_t *linear_hash
) {
	ion_status_t status = linear_hash_delete(key, linear_hash);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (err_ok == expected_status) {
		ion_byte_t *value = alloca(linear_hash->super.record.value_size);

		test_linear_hash_get(tc, key, err_item_not_found, 0, value, linear_hash);
	}
}

/**
@brief		Tests that the basic insert/update/delete/get operations of the linear_hash are working as intended
*/
void
test_linear_hash_basic_operations(
	planck_unit_test_t *tc
) {
	linear_hash_table_t *linear_hash = malloc(sizeof(linear_hash_table_t));

	test_linear_hash_setup(tc, linear_hash);
	test_linear_hash_insert(tc, IONIZE(17, int), IONIZE(19, int), err_ok, 1, boolean_true, linear_hash);
	test_linear_hash_update(tc, IONIZE(17, int), IONIZE(25, int), err_ok, 1, linear_hash);
	test_linear_hash_delete(tc, IONIZE(17, int), err_ok, 1, linear_hash);
	test_linear_hash_takedown(tc, linear_hash);
}

/**
@brief		Tests that the linear_hash.bucket_map receives a new head for the idx after creating an overflow bucket
*/
void
test_linear_hash_bucket_map_head_updates(
	planck_unit_test_t *tc
) {
	linear_hash_table_t *linear_hash = malloc(sizeof(linear_hash_table_t));

	test_linear_hash_setup(tc, linear_hash);

	ion_fpos_t expected_bucket_head = 0;

	/* base case -- assuming bucket 0 is inserted to */
	test_linear_hash_insert(tc, IONIZE(0, int), IONIZE(19, int), err_ok, 1, boolean_true, linear_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, expected_bucket_head == array_list_get(0, linear_hash->bucket_map));

	/* fill bucket 0 and cause the generation of an overflow */
	int i;

	for (i = 0; i < linear_hash->records_per_bucket; i++) {
		test_linear_hash_insert(tc, IONIZE(0, int), IONIZE(19, int), err_ok, 1, boolean_true, linear_hash);
	}

	/* test that the head of 0 index bucket linked list in the bucket map has changed */
	PLANCK_UNIT_ASSERT_TRUE(tc, expected_bucket_head != array_list_get(0, linear_hash->bucket_map));

	ion_fpos_t total_record_size = linear_hash->super.record.key_size + linear_hash->super.record.value_size + sizeof(ion_byte_t);

	/* test that the new head of 0 index bucket is at the correct location */
	expected_bucket_head = fseek(linear_hash->database, -4 * total_record_size - sizeof(linear_hash_bucket_t), SEEK_END);
	PLANCK_UNIT_ASSERT_TRUE(tc, expected_bucket_head != array_list_get(0, linear_hash->bucket_map));

	test_linear_hash_takedown(tc, linear_hash);
}

/**
@brief		Tests that the linear_hash.num_buckets increments when an insert occurs that pushes the linear_hash passed its split threshold
*/
void
test_linear_hash_increment_buckets(
	planck_unit_test_t *tc
) {
	linear_hash_table_t *linear_hash = malloc(sizeof(linear_hash_table_t));

	test_linear_hash_setup(tc, linear_hash);

	double split_cardinality	= linear_hash->records_per_bucket * linear_hash->num_buckets * linear_hash->split_threshold / 100;

	int original_number_buckets = linear_hash->num_buckets;
	int i;

	/* test while inserting to reach threshold - linear_hash.num_buckets should not change over this range */
	for (i = 0; i < split_cardinality; i++) {
		test_linear_hash_insert(tc, IONIZE(17, int), IONIZE(19, int), err_ok, 1, boolean_true, linear_hash);
		PLANCK_UNIT_ASSERT_TRUE(tc, original_number_buckets == linear_hash->num_buckets);
	}

	/* test inserting push above threshold - linear_hash.num_buckets should increase by one */
	test_linear_hash_insert(tc, IONIZE(17, int), IONIZE(19, int), err_ok, 1, boolean_true, linear_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, original_number_buckets + 1 == linear_hash->num_buckets);

	test_linear_hash_takedown(tc, linear_hash);
}

/**
@brief		Tests that the proper hash function is used for records that hash to a bucket that has been split
*/
void
test_linear_hash_correct_hash_function(
	planck_unit_test_t *tc
) {
	linear_hash_table_t *linear_hash = malloc(sizeof(linear_hash_table_t));

	test_linear_hash_setup(tc, linear_hash);

	double split_cardinality	= linear_hash->records_per_bucket * linear_hash->num_buckets * linear_hash->split_threshold / 100;

	/* assuming initial size of 5 so that key 5 hashes to bucket 0 using h0 */
	int expected_hash_bucket	= 0;

	int *k						= alloca(sizeof(int));

	*k = 5;

	ion_byte_t *hash_key = alloca(linear_hash->super.record.key_size);

	memcpy(hash_key, k, sizeof(linear_hash->super.record.key_size));

	/* resolve buck key 5 hashes to given the current linear_hash state - should be 0 */
	int hash_idx = insert_hash_to_bucket(hash_key, linear_hash);

	if (hash_idx < linear_hash->next_split) {
		hash_idx = hash_to_bucket(hash_key, linear_hash);
	}

	int i;

	PLANCK_UNIT_ASSERT_TRUE(tc, expected_hash_bucket == hash_idx);

	/* test while inserting to reach threshold - linear_hash.num_buckets should not change over this range */
	for (i = 0; i < split_cardinality; i++) {
		test_linear_hash_insert(tc, IONIZE(5, int), IONIZE(5, int), err_ok, 1, boolean_true, linear_hash);
	}

	/* test inserting push above threshold - linear_hash.num_buckets should increase by one */
	test_linear_hash_insert(tc, IONIZE(5, int), IONIZE(5, int), err_ok, 1, boolean_true, linear_hash);

	expected_hash_bucket	= 5;

	/* resolve buck key 5 hashes to given the current linear_hash state - should be 5 */
	hash_idx				= insert_hash_to_bucket(hash_key, linear_hash);

	if (hash_idx < linear_hash->next_split) {
		hash_idx = hash_to_bucket(hash_key, linear_hash);
	}

	/* assuming initial size of 5 so that key 5 hashes to bucket 5 using h1 */
	PLANCK_UNIT_ASSERT_TRUE(tc, expected_hash_bucket == hash_idx);

	test_linear_hash_takedown(tc, linear_hash);
}

/**
@brief		Tests that records are rehashed to the correct bucket after a split
*/
void
test_linear_hash_correct_bucket_after_split(
	planck_unit_test_t *tc
) {
	linear_hash_table_t *linear_hash = malloc(sizeof(linear_hash_table_t));

	test_linear_hash_setup(tc, linear_hash);

	double split_cardinality				= linear_hash->records_per_bucket * linear_hash->num_buckets * linear_hash->split_threshold / 100;

	/* assuming initial size of 5 so that key 5 hashes to bucket 0 using h0 */
	int			expected_hash_bucket		= 0;
	ion_fpos_t	expected_bucket_location	= array_list_get(expected_hash_bucket, linear_hash->bucket_map);

	int *k									= alloca(sizeof(int));

	*k = 5;

	ion_byte_t *hash_key = alloca(linear_hash->super.record.key_size);

	memcpy(hash_key, k, sizeof(linear_hash->super.record.key_size));

	/* resolve buck key 5 hashes to given the current linear_hash state - should be 0 */
	int hash_idx = insert_hash_to_bucket(hash_key, linear_hash);

	if (hash_idx < linear_hash->next_split) {
		hash_idx = hash_to_bucket(hash_key, linear_hash);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, expected_bucket_location == array_list_get(hash_idx, linear_hash->bucket_map));

	int i;

	/* test while inserting to reach threshold - linear_hash.num_buckets should not change over this range */
	for (i = 0; i < split_cardinality; i++) {
		test_linear_hash_insert(tc, IONIZE(5, int), IONIZE(5, int), err_ok, 1, boolean_true, linear_hash);
	}

	/* test inserting push above threshold - linear_hash.num_buckets should increase by one */
	test_linear_hash_insert(tc, IONIZE(5, int), IONIZE(5, int), err_ok, 1, boolean_true, linear_hash);

	expected_hash_bucket		= 5;
	expected_bucket_location	= array_list_get(expected_hash_bucket, linear_hash->bucket_map);

	/* resolve buck key 5 hashes to given the current linear_hash state - should be 5 */
	hash_idx					= insert_hash_to_bucket(hash_key, linear_hash);

	if (hash_idx < linear_hash->next_split) {
		hash_idx = hash_to_bucket(hash_key, linear_hash);
	}

	/* assuming initial size of 5 so that key 5 hashes to bucket 5 using h1 */
	PLANCK_UNIT_ASSERT_TRUE(tc, expected_bucket_location == array_list_get(hash_idx, linear_hash->bucket_map));
	test_linear_hash_takedown(tc, linear_hash);
}

/**
@brief		Tests that the number of records in the linear hash gets incremented and decremented on insertions and deletions respectvely
*/
void
test_linear_hash_global_record_increments_decrements(
	planck_unit_test_t *tc
) {
	linear_hash_table_t *linear_hash = malloc(sizeof(linear_hash_table_t));

	test_linear_hash_setup(tc, linear_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == linear_hash->num_records);
	test_linear_hash_insert(tc, IONIZE(17, int), IONIZE(19, int), err_ok, 1, boolean_true, linear_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == linear_hash->num_records);
	test_linear_hash_delete(tc, IONIZE(17, int), err_ok, 1, linear_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == linear_hash->num_records);
	test_linear_hash_takedown(tc, linear_hash);
}

/**
@brief		Tests that the number of records for a bucket gets incremented and decremented on insertions and deletions respectvely
*/
void
test_linear_hash_local_record_increments_decrements(
	planck_unit_test_t *tc
) {
	linear_hash_table_t *linear_hash = malloc(sizeof(linear_hash_table_t));

	test_linear_hash_setup(tc, linear_hash);

	linear_hash_bucket_t *bucket = alloca(sizeof(linear_hash_bucket_t));

	bucket->record_count		= -1;
	bucket->anchor_record		= -1;
	bucket->idx					= -1;
	bucket->overflow_location	= -1;

	linear_hash_get_bucket(0, bucket, linear_hash);

	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == bucket->record_count);
	test_linear_hash_insert(tc, IONIZE(0, int), IONIZE(19, int), err_ok, 1, boolean_true, linear_hash);
	linear_hash_get_bucket(0, bucket, linear_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == bucket->record_count);
	test_linear_hash_delete(tc, IONIZE(0, int), err_ok, 1, linear_hash);
	linear_hash_get_bucket(0, bucket, linear_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == bucket->record_count);

	test_linear_hash_takedown(tc, linear_hash);
}

/**
@brief		Tests some basic creation and destruction stuff for the flat file.
*/
void
test_linear_hash_create_destroy(
	planck_unit_test_t *tc
) {
	linear_hash_table_t *linear_hash	= malloc(sizeof(linear_hash_table_t));
	ion_err_t			err				= linear_hash_init(0, 4, key_type_numeric_unsigned, (int) sizeof(int), (int) sizeof(int), 5, 85, 4, linear_hash);

	linear_hash_destroy(linear_hash);
	free(linear_hash);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == 1);

/*  test_linear_hash_setup(tc, linear_hash); */
/*	test_linear_hash_takedown(tc, linear_hash); */
}

planck_unit_suite_t *
linear_hash_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_create_destroy);
	/*
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_basic_operations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_bucket_map_head_updates);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_increment_buckets);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_correct_hash_function);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_correct_bucket_after_split);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_global_record_increments_decrements);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_local_record_increments_decrements);
	 */

	return suite;
}

void
runalltests_linear_hash(
) {
	planck_unit_suite_t *suite = linear_hash_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
