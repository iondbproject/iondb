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

/**
 * Creates a linear hash table
 */
void
test_linear_hash_setup(planck_unit_test_t *tc, ion_linear_hash_table_t *lht) {
    ion_err_t err = ion_linear_hash_init(
            1,
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
 * Cleans up a linear hash table test
 */
void
test_linear_hash_tear_down(planck_unit_test_t *tc, ion_linear_hash_table_t *lht) {
    ion_err_t err = ion_linear_hash_close(lht);
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

//endregion


//region Get tests

void test_linear_hash_get_finds_from_an_overflow_bucket(planck_unit_test_t *tc) {
    ion_linear_hash_table_t table;
    test_linear_hash_setup(tc, &table);

    // Using the basic integer hashing
    int key = table.initial_size - 1;
    int total_buckets = table.total_buckets;

    for (int i = 0; i < table.records_per_bucket; i++) {
        test_linear_hash_insert(&table, tc, key, key, boolean_false);
    }
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, total_buckets, table.total_buckets)

    // Inserting another should make a new bucket and should be able to be retrieved.
    test_linear_hash_insert(&table, tc, key + table.initial_size, key * 2, boolean_true);
    PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, total_buckets + 1, table.total_buckets)

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

void test_linear_hash_deletes_from_overflow_bucket() {
    // Fill a bucket with data to generate an overflow bucket


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

    // Get specific tests
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_get_finds_from_an_overflow_bucket);


    // Delete specific tests
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_delete_decrements_records_count);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_delete);
    PLANCK_UNIT_ADD_TO_SUITE(suite, test_linear_hash_delete_updates_bucket_header);

    return suite;
}

void
runalltests_linear_hash(
) {
    planck_unit_suite_t *suite = linear_hash_getsuite();

    planck_unit_run_suite(suite);
    planck_unit_destroy_suite(suite);
}
