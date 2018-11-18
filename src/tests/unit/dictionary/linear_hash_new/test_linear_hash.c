/******************************************************************************/
/**
@file		test_linear_hash.c
@author		Andrew Feltham, Spencer MacBeth
@brief		Implementation unit tests for the linear hash.
@copyright	Copyright 2018
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#include "test_linear_hash.h"
#include "../../../../key_value/kv_system.h"
#include "../../../../dictionary/linear_hash_new/linear_hash.h"
#include "../../../../dictionary/dictionary.h"
#include <time.h>

unsigned int linear_hash_id = 0;

/**
 * Creates a linear hash table
 */
void
test_linear_hash_setup(planck_unit_test_t *tc, ion_linear_hash_table_t *lht) {
    ion_err_t err = ion_linear_hash_init(
            ++linear_hash_id,
            key_type_numeric_signed,
            sizeof(int),
            sizeof(int),
            4,
            85,
            lht
    );
    lht->super.compare = dictionary_compare_signed_value;
    lht->hash_key = ion_linear_hash_int_key_hash;
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
 * Creates a linear hash table
 */
void
test_linear_hash_setup_char_value(planck_unit_test_t *tc, ion_linear_hash_table_t *lht, ion_value_size_t value_size) {
    ion_err_t err = ion_linear_hash_init(
            ++linear_hash_id,
            key_type_numeric_signed,
            sizeof(int),
            value_size,
            4,
            85,
            lht
    );
    lht->super.compare = dictionary_compare_signed_value;
    lht->hash_key = ion_linear_hash_int_key_hash;
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
 * Creates a linear hash table
 */
void
test_linear_hash_setup_char_key_int_value(planck_unit_test_t *tc, ion_linear_hash_table_t *lht,
                                          ion_key_size_t key_size) {
    ion_err_t err = ion_linear_hash_init(
            ++linear_hash_id,
            key_type_char_array,
            key_size,
            sizeof(int),
            4,
            85,
            lht
    );
    lht->super.compare = dictionary_compare_unsigned_value;
    lht->hash_key = ion_linear_hash_int_key_hash;
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
 * Cleans up a linear hash table test
 */
void
test_linear_hash_tear_down(planck_unit_test_t *tc, ion_linear_hash_table_t *lht) {
    ion_err_t err = ion_linear_hash_close(lht);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
    err = ion_linear_hash_destroy_dictionary(lht->super.id);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
 * Helper method to read the first bucket from a file
 */
void
test_read_bucket_for_key(planck_unit_test_t *tc, ion_linear_hash_table_t *table, int key,
                         ion_byte_t *buffer) {
    int idx = ion_linear_hash_h0(key, table);
    int block = ion_array_list_get(idx, table->bucket_map);
    PLANCK_UNIT_ASSERT_INT_ARE_NOT_EQUAL(tc, -1, block)

    ion_err_t err = ion_linear_hash_read_block(block, table, buffer);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

ion_err_t
test_read_key_and_value_at_block_position(ion_linear_hash_table_t *table, ion_byte_t *buffer, ion_key_t out_key,
                                          ion_value_t out_value, int position) {
    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) buffer;
    if (position >= bucket->records || bucket->records == 0) {
        return err_out_of_bounds;
    }
    out_key = buffer + (table->record_total_size * position);
    out_value = buffer + (table->record_total_size * position) + table->super.record.key_size;
    return err_ok;
}

void
test_linear_hash_check_bucket_record_count(planck_unit_test_t *tc, ion_linear_hash_table_t *table, int block,
                                           int expected_count) {
    ion_err_t err = ion_linear_hash_read_block(block, table, table->block1);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)
    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) table->block1;
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, bucket->records)
}

/**
 * Helper method to insert a key and value into the linear hash table, check the insert status and optionally verify
 * the get can access that value.
 */
void test_linear_hash_insert(ion_linear_hash_table_t *lht, planck_unit_test_t *tc, int key, int value,
                             ion_boolean_t verify) {
    ion_status_t status = ion_linear_hash_insert(&key, &value, lht);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)

    int result_value = value - 1;
    if (boolean_true == verify) {
        status = ion_linear_hash_get(&key, &result_value, lht);
        PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
        PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
        PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, result_value, value);
    }
}

//region Insert tests

/**
 * Verifies that inserting a record increment the total records stored.
 */
void test_linear_hash_insert_increments_num_records(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    int expected = table.num_records + 1;
    int key = 1;
    int value = 2;
    ion_status_t status = ion_linear_hash_insert(&key, &value, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected, table.num_records);
    test_linear_hash_tear_down(tc, &table);
}

/**
 * Verifies that inserting a record increment the total records stored.
 */
void test_linear_hash_insert_increments_bucket_header_count(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    int key = 4;
    int value = 5;
    test_linear_hash_insert(&table, tc, key, value, boolean_true);

    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) table.block1;

    test_read_bucket_for_key(tc, &table, key, table.block1);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, bucket->records);
    test_linear_hash_tear_down(tc, &table);
}

/**
 * Verifies that an overflow bucket is created when a bucket becomes full
 */
void test_linear_hash_insert_creates_overflow_bucket(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);
    int inserted = 0;
    int key = 0;
    int expected_blocks = table.total_buckets + 1;
    while (inserted <= table.records_per_bucket) {
        if (ion_linear_hash_h0(key, &table) == 0) {
            test_linear_hash_insert(&table, tc, key, key, boolean_true);
            inserted++;
        }
        key++;
    }

    // Verify that the total bucket count increased.
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_blocks, table.total_buckets);

    // Read the bucket for the block and make sure that it has 1 record and an overflow
    int block = ion_array_list_get(0, table.bucket_map);
    ion_err_t err = ion_linear_hash_read_block(block, &table, table.block1);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) table.block1;

    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, bucket->records);
    PLANCK_UNIT_ASSERT_INT_ARE_NOT_EQUAL(tc, LINEAR_HASH_NO_OVERFLOW, bucket->overflow_block);

    // Read the bucket for the overflow block and ensure that it has the full amount of records
    block = bucket->overflow_block;
    ion_linear_hash_read_block(block, &table, table.block1);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, table.records_per_bucket, bucket->records);

    test_linear_hash_tear_down(tc, &table);
}

void
test_linear_hash_insert_adds_the_new_bucket_to_the_bucket_map(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);
    int expected_total_buckets = table.total_buckets + 1;
    int expected_block_index = table.next_block;
    int current_block_for_idx = ion_array_list_get(0, table.bucket_map);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, current_block_for_idx)

    int count = 0;
    int num = table.records_per_bucket + 1;
    int i = 0;
    while (count < num) {
        test_linear_hash_insert(&table, tc, i, i, boolean_false);
        i += table.initial_size;
        count++;
    }

    test_linear_hash_check_bucket_record_count(tc, &table, 0, table.records_per_bucket);

    current_block_for_idx = ion_array_list_get(0, table.bucket_map);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_block_index, current_block_for_idx)

    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_total_buckets, table.total_buckets)

    test_linear_hash_tear_down(tc, &table);
}

//endregion


//region Get tests

void test_linear_hash_get_finds_from_an_overflow_bucket(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    // Using the basic integer hashing
    int key = table.initial_size - 1;
    int total_buckets = table.total_buckets;

    for (int i = 0; i < table.records_per_bucket; ++i) {
        test_linear_hash_insert(&table, tc, key, key, boolean_false);
    }
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, total_buckets, table.total_buckets)

    // Inserting another should make a new bucket and should be able to be retrieved.
    test_linear_hash_insert(&table, tc, key + table.initial_size, key * 2, boolean_true);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, total_buckets + 1, table.total_buckets)

    test_linear_hash_tear_down(tc, &table);
}

/**
 * Tests that a int key with a char value can be retrieved.
 */
void test_linear_hash_get_char_value(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    const int size = 32;
    char str[size];
    strncpy(str, "Test This Thing", size);
    int key = 654;
    test_linear_hash_setup_char_value(tc, &table, (ion_value_size_t) size);

    ion_status_t status = ion_linear_hash_insert(&key, &str, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)

    char result[size];
    status = ion_linear_hash_get(&key, &result, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
    PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, str, result)

    test_linear_hash_tear_down(tc, &table);
}

/**
 * Tests that a int key with a char value can be retrieved.
 */
void test_linear_hash_get_char_key(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    const int size = 32;
    char str[size];
    strncpy(str, "Test This Thing", size);
    int value = 654;
    test_linear_hash_setup_char_key_int_value(tc, &table, (ion_key_size_t) size);

    ion_status_t status = ion_linear_hash_insert(&str, &value, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)

    int result = 0;
    status = ion_linear_hash_get(&str, &result, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value, result)

    test_linear_hash_tear_down(tc, &table);
}

//endregion


//region Delete tests

/**
 * Verifies that a record can be deleted
 */
void test_linear_hash_delete(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);
    int key = 1;
    int value = 2;
    test_linear_hash_insert(&table, tc, key, value, boolean_true);
    ion_status_t status = ion_linear_hash_delete(&key, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
    test_linear_hash_tear_down(tc, &table);
}

void test_linear_hash_delete_updates_bucket_header(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);
    int key = 1;
    int value = 2;
    test_linear_hash_insert(&table, tc, key, value, boolean_true);
    ion_status_t status = ion_linear_hash_delete(&key, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)

    // Verify that the block header is now 0
    test_read_bucket_for_key(tc, &table, key, table.block1);
    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) table.block1;
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, bucket->records)
    test_linear_hash_tear_down(tc, &table);
}

/**
 * Verifies that deleting a record decrements the total number of records
 */
void test_linear_hash_delete_decrements_records_count(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    int expected = table.num_records;
    int key = 1;
    int value = 2;
    test_linear_hash_insert(&table, tc, key, value, boolean_true);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected + 1, table.num_records)

    ion_status_t status = ion_linear_hash_delete(&key, &table);

    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected, table.num_records)

    test_linear_hash_tear_down(tc, &table);
}

void test_linear_hash_delete_and_fills_gaps(planck_unit_test_t *tc) {
    // Fill a bucket with data to generate an overflow bucket
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    // Fill the table with 3 values.
    int key = 0;
    int key2 = key + table.initial_size;
    int key3 = key2 + table.initial_size;
    test_linear_hash_insert(&table, tc, key, key, boolean_true);
    test_linear_hash_insert(&table, tc, key2, key2, boolean_true);
    test_linear_hash_insert(&table, tc, key3, key3, boolean_true);


    // Delete the gap
    ion_status_t status = ion_linear_hash_delete(IONIZE(key2, int), &table);

    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)

    // Verify we can retrieve the other items
    int value = -1;
    status = ion_linear_hash_get(&key, &value, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key, value)

    status = ion_linear_hash_get(&key3, &value, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key3, value)

    status = ion_linear_hash_get(&key2, &value, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, status.count)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_item_not_found, status.error)

    test_linear_hash_tear_down(tc, &table);
}

void test_linear_hash_deletes_from_overflow_bucket(planck_unit_test_t *tc) {
    // Fill a bucket with data to generate an overflow bucket
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    int key = table.initial_size - 2;
    int buckets = table.total_buckets;
    for (int i = 0; i < table.records_per_bucket; ++i) {
        test_linear_hash_insert(&table, tc, key, key, boolean_true);
    }

    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, buckets, table.total_buckets)
    int key2 = key + table.initial_size;
    int key_h0 = ion_linear_hash_h0(key, &table);
    int key2_h0 = ion_linear_hash_h0(key2, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_h0, key2_h0)

    test_linear_hash_insert(&table, tc, key2, key2, boolean_true);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, buckets + 1, table.total_buckets)

    ion_status_t result = ion_linear_hash_delete(&key2, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, result.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, result.count)
    int value = 0;
    ion_status_t status = ion_linear_hash_get(&key2, &value, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_item_not_found, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, status.count)

    test_linear_hash_tear_down(tc, &table);
}

void test_linear_hash_delete_char_key(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    const int size = 32;
    char str[size];
    strncpy(str, "Test This Thing", size);
    int value = 654;
    test_linear_hash_setup_char_key_int_value(tc, &table, (ion_key_size_t) size);

    ion_status_t status = ion_linear_hash_insert(&str, &value, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)

    status = ion_linear_hash_delete(&str, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)

    int result = 0;
    status = ion_linear_hash_get(&str, &result, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_item_not_found, status.error)

    test_linear_hash_tear_down(tc, &table);
}

//endregion


//region splitting

void
test_linear_hash_split_adds_the_new_bucket_to_the_bucket_map(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);
    int split_idx = table.current_size;
    int expected_block_index = table.next_block;
    int expected_total_buckets = table.total_buckets + 1;

    int current_block_for_idx = ion_array_list_get(split_idx, table.bucket_map);
    PLANCK_UNIT_ASSERT_INT_ARE_NOT_EQUAL(tc, expected_block_index, current_block_for_idx)

    ion_err_t err = ion_linear_hash_split(&table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)

    current_block_for_idx = ion_array_list_get(split_idx, table.bucket_map);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_block_index, current_block_for_idx)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_total_buckets, table.total_buckets)

    test_linear_hash_tear_down(tc, &table);
}

void
test_linear_hash_triggers_a_split_at_the_threshold(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);
    int split_records =
            (int) (table.records_per_bucket * table.current_size * (table.split_threshold / 100.0));
    int current_size = table.current_size;
    int expected_size = table.current_size + 1;
    int i;
    for (i = 0; i < split_records; ++i) {
        test_linear_hash_insert(&table, tc, i, i, boolean_true);
    }

    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, current_size, table.current_size)

    // Trigger a split with the insert record
    test_linear_hash_insert(&table, tc, i, i, boolean_true);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_size, table.current_size)

    test_linear_hash_tear_down(tc, &table);
}

/**
 * A simple test with two values in a bucket. One should be move, the other should not
 *  */
void
test_linear_hash_split_move_item_to_new_bucket(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    int value1 = 26;
    int value2 = 80;
    int key1 = 0;
    int key2 = 4;
    int split_idx_block = table.next_block;
    test_linear_hash_insert(&table, tc, key1, value1, boolean_true);
    test_linear_hash_insert(&table, tc, key2, value2, boolean_true);
    test_linear_hash_check_bucket_record_count(tc, &table, 0, 2);

    // Trigger a split
    ion_err_t err = ion_linear_hash_split(&table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)

    // Check the original bucket contains 1 record.
    ion_linear_hash_read_block(0, &table, table.block1);
    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) table.block1;
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, bucket->records)

    // Check the first record still exists in the original bucket.
    int *key = (int *) (table.block1 + sizeof(ion_linear_hash_bucket_t));
    int *value = (int *) (table.block1 + sizeof(ion_linear_hash_bucket_t) + table.super.record.key_size);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key1, *key)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value1, *value)

    // Verify that the new bucket contains the record and has a total of one record
    ion_linear_hash_read_block(split_idx_block, &table, table.block1);
    bucket = (ion_linear_hash_bucket_t *) table.block1;
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, bucket->records)

    // Check the first record
    key = (int *) (table.block1 + sizeof(ion_linear_hash_bucket_t));
    value = (int *) (table.block1 + sizeof(ion_linear_hash_bucket_t) + table.super.record.key_size);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key2, *key)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value2, *value)

    test_linear_hash_tear_down(tc, &table);
}

/**
 * A simple test with two values in a bucket where one should move and the other should not.
 *  */
void
test_linear_hash_split_verify_get_retrieves_items(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    int value1 = 26;
    int value2 = 80;
    int key1 = 0;
    int key2 = 4;
    test_linear_hash_insert(&table, tc, key1, value1, boolean_true);
    test_linear_hash_insert(&table, tc, key2, value2, boolean_true);
    test_linear_hash_check_bucket_record_count(tc, &table, 0, 2);

    // Trigger a split
    ion_err_t err = ion_linear_hash_split(&table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)

    // Verify that get still returns the items
    int value = 0;
    ion_status_t status = ion_linear_hash_get(&key1, &value, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value1, value)

    status = ion_linear_hash_get(&key2, &value, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value2, value)

    test_linear_hash_tear_down(tc, &table);
}

/**
 * A simple test with two values in a bucket. One should be move, the other should not
 *  */
void
test_linear_hash_moves_records_in_buckets_to_fill_space(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    int key2 = 4;
    int key1 = 0;
    int value1 = 12312;
    int value2 = 858343;
    test_linear_hash_insert(&table, tc, key2, value2, boolean_true);
    test_linear_hash_insert(&table, tc, key1, value1, boolean_true);
    test_linear_hash_check_bucket_record_count(tc, &table, 0, 2);

    // Trigger a split
    ion_err_t err = ion_linear_hash_split(&table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)

    // Verify that get still returns the items
    int value = 0;
    ion_status_t status = ion_linear_hash_get(&key1, &value, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value1, value)

    status = ion_linear_hash_get(&key2, &value, &table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value2, value)

    test_linear_hash_tear_down(tc, &table);
}

/**
 * Tests that overflow buckets are created by filling the first bucket and one of it's overflow entirely with values
 * that will be split to the new bucket, triggering a split and checking the resulting new buckets.
 */
void
test_linear_hash_split_creates_overflow_buckets(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);
    int count = 0;
    int num = table.records_per_bucket + 1;
    int split_idx = table.current_size;
    int i = 0;
    while (count < num) {
        int h0 = ion_linear_hash_h0(i, &table);
        int h1 = ion_linear_hash_h1(i, &table);
        if (h1 != h0 && table.initial_size == h1) {
            test_linear_hash_insert(&table, tc, i, i, boolean_false);
            count++;
        }
        i++;
    }

    // Verify that num records exist
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, num, table.num_records)

    // Trigger a split
    ion_linear_hash_split(&table);

    // Check the first bucket, it should be empty
    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) table.block1;
    ion_err_t err;
    int block = ion_array_list_get(0, table.bucket_map);
    PLANCK_UNIT_ASSERT_INT_ARE_NOT_EQUAL(tc, -1, block);
    err = ion_linear_hash_read_block(block, &table, table.block1);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, bucket->records)

    // Check the old overflow, it should also be empty
    err = ion_linear_hash_read_block(bucket->overflow_block, &table, table.block1);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, bucket->records)

    // Check the new bucket, it should contain 1 record from the old overflow
    block = ion_array_list_get(split_idx, table.bucket_map);
    PLANCK_UNIT_ASSERT_INT_ARE_NOT_EQUAL(tc, -1, block);
    err = ion_linear_hash_read_block(block, &table, table.block1);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, bucket->records)

    // Check the new overflow, it should be full
    err = ion_linear_hash_read_block(bucket->overflow_block, &table, table.block1);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, table.records_per_bucket, bucket->records)

    test_linear_hash_tear_down(tc, &table);
}

void
test_linear_hash_split_keeps_the_same_record_count(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    int num_records = 20;
    // Insert some random data
    for (int i = 0; i < num_records; ++i) {
        test_linear_hash_insert(&table, tc, i, i, boolean_true);
    }
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, num_records, table.num_records)
    ion_err_t err = ion_linear_hash_split(&table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, num_records, table.num_records);
    test_linear_hash_tear_down(tc, &table);
}

void
test_linear_hash_split_adds_a_new_bucket(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);
    int current_size = table.current_size;
    int current_block = table.next_block;
    ion_err_t err = ion_linear_hash_split(&table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, current_size + 1, table.current_size);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, current_block + 1, table.next_block);
    test_linear_hash_tear_down(tc, &table);
}

void
test_linear_hash_split_increments_the_split(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);
    int current_split = table.next_split;
    ion_err_t err = ion_linear_hash_split(&table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, current_split + 1, table.next_split);
    test_linear_hash_tear_down(tc, &table);
}

void
test_linear_hash_split_resets_when_the_size_doubles(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);
    table.current_size = table.initial_size * 2 - 1;
    int expected_split = 0;
    table.next_split = table.initial_size - 1;
    ion_err_t err = ion_linear_hash_split(&table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_split, table.next_split);
    test_linear_hash_tear_down(tc, &table);
}

void
test_linear_hash_split_increments_the_initial_size_when_doubled(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);
    table.current_size = table.initial_size * 2 - 1;
    int expected_size = table.initial_size * 2;
    table.next_split = table.initial_size - 1;
    ion_err_t err = ion_linear_hash_split(&table);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_size, table.initial_size);
    test_linear_hash_tear_down(tc, &table);
}
//endregion

//region Save and restore

void test_linear_hash_can_save_and_restore_records(planck_unit_test_t *tc) {
    //Creates an instance of a linear hash
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    int keys[100];
    int values[100];

    for (int i = 0; i < 100; ++i) {
        test_linear_hash_insert(&table, tc, i, i, boolean_true);
        keys[i] = i;
        values[i] = i;
    }
    ion_dictionary_id_t id = table.super.id;
    ion_err_t err = ion_linear_hash_close(&table);

    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)

    ion_linear_hash_table_t new_table;
    err = ion_linear_hash_init(
            id,
            key_type_numeric_signed,
            sizeof(int),
            sizeof(int),
            4,
            85,
            &new_table
    );
    new_table.super.compare = dictionary_compare_signed_value;

    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)

    int value = -1;
    ion_status_t status;
    for (int i = 0; i < 100; ++i) {
        status = ion_linear_hash_get(IONIZE(i, int), IONIZE(value, int), &new_table);
        PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error)
        PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count)
    }

    test_linear_hash_tear_down(tc, &new_table);
}
//endregion

//region arraylist

void
test_linear_hash_array_list_can_be_saved_and_restored(planck_unit_test_t *tc) {
    ion_array_list_t list;
    ion_err_t err;
    err = ion_array_list_init(100, &list);

    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)

    int values[100];
    for (int i = 0; i < 100; i++) {
        err = ion_array_list_insert(i, i, &list);
        PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)
        values[i] = i;
    }

    FILE *file = fopen("arraylist.dat", "w+b");
    err = ion_array_list_save_to_file(file, &list);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)
    ion_array_list_destroy(&list);

    ion_array_list_t restored;
    fseek(file, 0, SEEK_SET);
    err = ion_array_list_init_from_file(file, &restored);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)

    for (int i = 0; i < 100; i++) {
        int result = ion_array_list_get(i, &restored);
        PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, values[i], result)
    }

    ion_array_list_destroy(&restored);
}

void
test_linear_hash_array_list_destroy_frees_memory(planck_unit_test_t *tc) {
    ion_array_list_t list;
    ion_err_t err;
    err = ion_array_list_init(100, &list);

    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)

    int values[100];
    for (int i = 0; i < 100; i++) {
        err = ion_array_list_insert(i, i, &list);
        PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err)
        values[i] = i;
    }
    ion_array_list_destroy(&list);
    PLANCK_UNIT_ASSERT_TRUE(tc, NULL == list.data)
}

//endregion

planck_unit_suite_t *
linear_hash_getsuite(
) {
    planck_unit_suite_t *suite = planck_unit_new_suite();

    // Insert specific tests
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_insert_increments_bucket_header_count);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_insert_creates_overflow_bucket);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_insert_increments_num_records);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_insert_adds_the_new_bucket_to_the_bucket_map);

    // Get specific tests
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_get_finds_from_an_overflow_bucket);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_get_char_value);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_get_char_key);


    // Delete specific tests
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_delete_decrements_records_count);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_delete);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_delete_updates_bucket_header);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_deletes_from_overflow_bucket);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_delete_and_fills_gaps);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_delete_char_key);

    // Splitting
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_split_increments_the_split);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_split_resets_when_the_size_doubles);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_split_increments_the_initial_size_when_doubled);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_split_adds_a_new_bucket);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_split_creates_overflow_buckets);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_split_adds_the_new_bucket_to_the_bucket_map);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_split_keeps_the_same_record_count);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_split_move_item_to_new_bucket);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_split_verify_get_retrieves_items);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_moves_records_in_buckets_to_fill_space);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_triggers_a_split_at_the_threshold);

    // Saving and restoring
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_can_save_and_restore_records);

    // Array List
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_array_list_can_be_saved_and_restored);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_array_list_destroy_frees_memory);

    return suite;
}

void
runalltests_linear_hash(
) {
    planck_unit_suite_t *suite = linear_hash_getsuite();

    planck_unit_run_suite(suite);
    planck_unit_destroy_suite(suite);
}
