/******************************************************************************/
/**
@file		linear_hash.c
@author		Andrew Feltham, Spencer MacBeth
			All rights reserved.
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

#include "linear_hash.h"
#include "../../key_value/kv_system.h"
#include <stdio.h>
#include "../../file/kv_stdio_intercept.h"

#if LINEAR_HASH_DEBUG > 0

void
ion_linear_hash_debug_print_key(
        ion_key_t key,
        ion_linear_hash_table_t *lht
) {
    if (key_type_numeric_signed == lht->super.key_type) {
        printf("%i", *(int *) key);
    } else if (key_type_null_terminated_string == lht->super.key_type) {
        printf("%s", (char *) key);
    }
}

void
ion_linear_hash_debug_print_bucket(
        ion_linear_hash_bucket_t *bucket
) {
    printf("Bucket %d: record count: %d, overflow: %d, size: %u\n", bucket->block, bucket->records,
           bucket->overflow_block,
           sizeof(ion_linear_hash_bucket_t));
}

#endif

void
ion_linear_hash_initialize_bucket(
        int block_num,
        ion_byte_t *block,
        int block_size,
        int overflow_block
) {
    memset(block, 0, (size_t) block_size);
    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) block;
    bucket->block = block_num;
    bucket->overflow_block = overflow_block;
    bucket->records = 0;
}

/**
 * @brief Writes a bucket block to the database file, overwriting the current block
 * @param [in] bucket The bucket to write
 * @param [in] block The block number to write
 * @param [in] linear_hash The linear hash instance containing the database to use
 * @return The error (if any)
 */
ion_err_t
ion_linear_hash_write_bucket(ion_byte_t *bucket, int block, ion_linear_hash_table_t *linear_hash) {
    if (0 != fseek(linear_hash->database, block * LINEAR_HASH_BLOCK_SIZE, SEEK_SET)) {
        return err_file_bad_seek;
    }

    if (1 != fwrite(bucket, LINEAR_HASH_BLOCK_SIZE, 1, linear_hash->database)) {
        return err_file_write_error;
    }
    return err_ok;
}

ion_err_t
ion_linear_hash_init(
        ion_dictionary_id_t id,
        ion_dictionary_size_t dictionary_size,
        ion_key_type_t key_type,
        ion_key_size_t key_size,
        ion_value_size_t value_size,
        int initial_size,
        int split_threshold,
        int records_per_bucket,
        ion_linear_hash_table_t *linear_hash
) {
    linear_hash->super.id = id;
    linear_hash->dictionary_size = dictionary_size;
    linear_hash->super.key_type = key_type;
    linear_hash->super.record.key_size = key_size;
    linear_hash->super.record.value_size = value_size;

    /* initialize linear_hash fields */
    linear_hash->initial_size = initial_size;
    linear_hash->num_buckets = initial_size;
    linear_hash->num_records = 0;
    linear_hash->next_split = 0;
    linear_hash->split_threshold = split_threshold;
    linear_hash->record_total_size = key_size + value_size;
    linear_hash->records_per_bucket =
            (LINEAR_HASH_BLOCK_SIZE - sizeof(ion_linear_hash_bucket_t)) / (key_size + value_size);
    linear_hash->block1 = malloc(LINEAR_HASH_BLOCK_SIZE);
    linear_hash->block2 = malloc(LINEAR_HASH_BLOCK_SIZE);

    if (linear_hash->block1 == NULL || linear_hash->block2 == NULL) {
        //TODO Clean up
        return err_out_of_memory;
    }

    ion_array_list_t *bucket_map;
    bucket_map = malloc(sizeof(ion_array_list_t));
    ion_array_list_init(2 * initial_size, bucket_map);
    linear_hash->bucket_map = bucket_map;

    linear_hash->database = fopen("db.lhd", "w+");

#if LINEAR_HASH_DEBUG > 0
    printf("Initialized with %d records per bucket", linear_hash->records_per_bucket);
#endif

    // Create initial blocks
    for (int i = 0; i < 1; i++) {
        ion_linear_hash_initialize_bucket(i, linear_hash->block1, LINEAR_HASH_BLOCK_SIZE, LINEAR_HASH_NO_OVERFLOW);
        ion_linear_hash_write_bucket(linear_hash->block1, i, linear_hash);
        ion_array_list_insert(i, i, bucket_map);
    }

    return err_ok;
}

//region Bucket Index Hashing

/**
@brief		Transform a key to an integer.
@details	Applies a polynomial hash to the byte-string representation of the key to transform the key to an integer.
@param[in]	key
				Pointer to the key to hash
@param[in]	linear_hash
				Pointer to a linear hash table instance.
@return		The result of applying the polynomial hash to the key as an integer.
*/
int
ion_linear_hash_key_bytes_to_int(
        ion_byte_t *key,
        ion_linear_hash_table_t *linear_hash
) {
    int i;
    int key_bytes_as_int = 0;
    static int coefficients[] = {3, 5, 7, 11, 13, 17, 19};

    for (i = 0; i < linear_hash->super.record.key_size - 1; i++) {
        key_bytes_as_int += *(key + i) * coefficients[i + 1] - *(key + i) * coefficients[i];
    }

    return key_bytes_as_int;
}

int ion_linear_hash_h0(ion_byte_t *key, ion_linear_hash_table_t *linear_hash) {
    int key_bytes_as_int = ion_linear_hash_key_bytes_to_int(key, linear_hash);
    return key_bytes_as_int & (linear_hash->initial_size - 1);
}

int ion_linear_hash_h1(ion_byte_t *key, ion_linear_hash_table_t *linear_hash) {
    int key_bytes_as_int = ion_linear_hash_key_bytes_to_int(key, linear_hash);
    return key_bytes_as_int & ((2 * linear_hash->initial_size) - 1);
}
//endregion


//region File writing
/**
 * @brief Reads a bucket block from the database
 * @param [in] block [in] The integer block number to read
 * @param [in] linear_hash The linear hash instance containing the database to use
 * @param [out] bucket A buffer of size for a block that the bucket block will be read into
 * @return The error (if any)
 */
ion_err_t
ion_linear_hash_read_bucket(int block, ion_linear_hash_table_t *linear_hash, ion_byte_t *bucket) {
    if (0 != fseek(linear_hash->database, 0, SEEK_SET)) {
        return err_file_bad_seek;
    }

    if (1 != fread(bucket, LINEAR_HASH_BLOCK_SIZE, 1, linear_hash->database)) {
        return err_file_read_error;
    }
    return err_ok;
}

//endregion

ion_status_t ion_linear_hash_insert(
        ion_key_t key,
        ion_value_t value,
        ion_linear_hash_table_t *linear_hash
) {

#if LINEAR_HASH_DEBUG > 0
    printf("********** Insert **********\n");
    printf("Inserting key: ");
    ion_linear_hash_debug_print_key(key, linear_hash);
    printf("\n");
#endif

    int idx = ion_linear_hash_h0(key, linear_hash);
    if (idx < linear_hash->next_split) {
        idx = ion_linear_hash_h1(key, linear_hash);
    }

#if LINEAR_HASH_DEBUG > 0
    printf("Inserting with index %i\n", idx);
#endif

    int block = ion_array_list_get(idx, linear_hash->bucket_map);

    // Invalid block index (shouldn't happen)
    if (ARRAY_LIST_END_OF_LIST == block) {
        return ION_STATUS_ERROR(err_out_of_bounds);
    }

#if LINEAR_HASH_DEBUG > 0
    printf("Using block %d\n", block);
#endif

    ion_byte_t *buffer = malloc(LINEAR_HASH_BLOCK_SIZE);
    memset(buffer, 0, LINEAR_HASH_BLOCK_SIZE);
    // Read the block from the file
    ion_err_t err = ion_linear_hash_read_bucket(block, linear_hash, buffer);
    if (err_ok != err) {
        return ION_STATUS_ERROR(err);
    }
    ion_linear_hash_bucket_t *bucket;
    bucket = buffer;

#if LINEAR_HASH_DEBUG > 0
    ion_linear_hash_debug_print_bucket(bucket);
#endif


    if (bucket->records == linear_hash->records_per_bucket) {
        // Create Overflow bucket
    } else {
        int record_size = linear_hash->super.record.key_size + linear_hash->super.record.value_size;
        int key_offset = sizeof(ion_linear_hash_bucket_t) + record_size * bucket->records;
        int value_offset = key_offset + linear_hash->super.record.value_size;
#if LINEAR_HASH_DEBUG > 0
        printf("Inserting key at offset %d, value at offset: %d\n", key_offset, value_offset);
#endif
        memcpy(buffer + key_offset, key, (size_t) linear_hash->super.record.key_size);
        memcpy(buffer + value_offset, value, (size_t) linear_hash->super.record.value_size);
        bucket->records++;
        linear_hash->num_records++;

#if LINEAR_HASH_DEBUG > 0
        ion_key_t written_key;
        written_key = buffer + key_offset;
        printf("Written key ");
        ion_linear_hash_debug_print_key(written_key, linear_hash);
        printf("\n");
#endif
        ion_linear_hash_write_bucket(buffer, bucket->block, linear_hash);
    }
    free(buffer);
    return ION_STATUS_OK(1);
}

ion_status_t
ion_linear_hash_get(ion_key_t key, ion_value_t value, ion_linear_hash_table_t *linear_hash) {

#if LINEAR_HASH_DEBUG > 0
    printf("\n********** Get **********\n");
    printf("Getting with key: ");
    ion_linear_hash_debug_print_key(key, linear_hash);
    printf("\n");
#endif

    // Calculate the block index
    int idx = ion_linear_hash_h0((ion_byte_t *) key, linear_hash);

    if (idx < linear_hash->next_split) {
        idx = ion_linear_hash_h1((ion_byte_t *) key, linear_hash);
    }

#if LINEAR_HASH_DEBUG > 0
    printf("Hash index: %i\n", idx);
#endif
    unsigned long s = 512;
    ion_byte_t *buffer = malloc(LINEAR_HASH_BLOCK_SIZE);
    memset(buffer, 0, LINEAR_HASH_BLOCK_SIZE);
    // Read the first bucket
    int block = ion_array_list_get(idx, linear_hash->bucket_map);

    ion_err_t read_bucket = ion_linear_hash_read_bucket(block, linear_hash, buffer);
    if (err_ok != read_bucket) {
        return ION_STATUS_ERROR(read_bucket);
    }

#if LINEAR_HASH_DEBUG > 0
    printf("Using block %d\n", block);
#endif

    ion_linear_hash_bucket_t *bucket;
    bucket = (ion_linear_hash_bucket_t *) buffer;

#if LINEAR_HASH_DEBUG > 0
    ion_linear_hash_debug_print_bucket(bucket);
#endif

    // Pointers to the records
    ion_key_t *record_key = alloca(linear_hash->super.record.key_size);
    ion_value_t *record_value = alloca(linear_hash->super.record.value_size);

    size_t offset = sizeof(ion_linear_hash_bucket_t);
    ion_boolean_t terminal = boolean_true;
    int i;
    while (terminal) {
#if LINEAR_HASH_DEBUG > 0
        printf("Reading records\n");
#endif
        for (i = 0; i < bucket->records; i = i + 1) {

            memcpy(record_key, buffer + offset, (size_t) linear_hash->super.record.key_size);
            memcpy(record_value, buffer + offset + linear_hash->super.record.key_size,
                   (size_t) linear_hash->super.record.value_size);

#if LINEAR_HASH_DEBUG > 0
            printf("Checking record %d with key ", i);
            ion_linear_hash_debug_print_key(record_key, linear_hash);
            printf("\n");
#endif

            if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
                memcpy(value, record_value, (size_t) linear_hash->super.record.value_size);
                free(buffer);
                return ION_STATUS_OK(1);
            }
            offset += linear_hash->record_total_size;
        }

#if LINEAR_HASH_DEBUG > 0
        printf("Done checking this bucket\n");
#endif

        if (LINEAR_HASH_NO_OVERFLOW == bucket->overflow_block) {
            terminal = boolean_false;
        } else {
            ion_linear_hash_read_bucket(bucket->overflow_block, linear_hash, linear_hash->block1);
            offset = sizeof(bucket);
            i = 0;
        }
    }
#if LINEAR_HASH_DEBUG > 0
    printf("Done");
#endif
    free(buffer);
    return ION_STATUS_ERROR(err_item_not_found);
}
