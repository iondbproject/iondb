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

/**
 * Initializes the memory block to a new, empty block with no records.
 * @param block_num
 * @param block
 * @param block_size
 * @param overflow_block
 */
void
ion_linear_hash_initialize_bucket(
        int block_num,
        ion_byte_t *block,
        int block_size,
        int overflow_block
) {
    memset(block, 0, (size_t) block_size);
    ion_linear_hash_bucket_t *bucket;
    bucket = (ion_linear_hash_bucket_t *) block;
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
ion_linear_hash_write_block(ion_byte_t *bucket, int block, ion_linear_hash_table_t *linear_hash) {
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
        ion_linear_hash_table_t *linear_hash
) {
    linear_hash->super.id = id;
    linear_hash->dictionary_size = dictionary_size;
    linear_hash->super.key_type = key_type;
    linear_hash->super.record.key_size = key_size;
    linear_hash->super.record.value_size = value_size;

    /* initialize linear_hash fields */
    linear_hash->initial_size = initial_size;
    linear_hash->num_buckets = 0;
    linear_hash->num_records = 0;
    linear_hash->next_split = 0;
    linear_hash->total_blocks = 0;
    linear_hash->split_threshold = split_threshold;
    linear_hash->record_total_size = (unsigned int) (key_size + value_size);
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

    //TODO: Read and write state

    linear_hash->database = fopen("db.lhd", "w+");

#if LINEAR_HASH_DEBUG > 0
    printf("Initialized with %d records per bucket", linear_hash->records_per_bucket);
#endif

    // Create initial blocks
    for (int i = 0; i < initial_size; i++) {
        ion_linear_hash_initialize_bucket(i, linear_hash->block1, LINEAR_HASH_BLOCK_SIZE, LINEAR_HASH_NO_OVERFLOW);
        ion_linear_hash_write_block(linear_hash->block1, i, linear_hash);
        ion_array_list_insert(i, i, bucket_map);
        linear_hash->total_blocks++;
        linear_hash->num_buckets++;
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
#if LINEAR_HASH_DEBUG > 0
    printf("Hash value: %d\n", key_bytes_as_int);
#endif
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
ion_linear_hash_read_block(int block, ion_linear_hash_table_t *linear_hash, ion_byte_t *bucket) {
    if (0 != fseek(linear_hash->database, block * LINEAR_HASH_BLOCK_SIZE, SEEK_SET)) {
        return err_file_bad_seek;
    }

    if (1 != fread(bucket, LINEAR_HASH_BLOCK_SIZE, 1, linear_hash->database)) {
        return err_file_read_error;
    }
    return err_ok;
}

//endregion

ion_err_t ion_linear_hash_split(ion_linear_hash_table_t *lht) {

    int current_split = lht->next_split;
    int starting_blocks = lht->total_blocks;
    // Load the bucket to split
    int block = ion_array_list_get(current_split, lht->bucket_map);
    ion_byte_t *current_buffer = lht->block1;
    ion_err_t err = ion_linear_hash_read_block(block, lht, current_buffer);
    ion_linear_hash_bucket_t *current_bucket = (ion_linear_hash_bucket_t *) current_buffer;

    if (err_ok != err) {
        return err;
    }

    // Initialize a new bucket at the next available block
    ion_byte_t *new_buffer = lht->block2;
    ion_linear_hash_initialize_bucket(lht->total_blocks, new_buffer, LINEAR_HASH_BLOCK_SIZE, LINEAR_HASH_NO_OVERFLOW);
    ion_linear_hash_bucket_t *new_bucket = (ion_linear_hash_bucket_t *) current_buffer;

    lht->total_blocks++;


    ion_boolean_t completed = boolean_false;
    ion_byte_t *insert_ptr_new = new_buffer + sizeof(ion_linear_hash_bucket_t);
    ion_byte_t *insert_ptr_current = current_bucket + sizeof(ion_linear_hash_bucket_t);
    ion_byte_t *read_ptr;
    ion_byte_t *key;
    int num_records = current_bucket->records;

    while (!completed) {
        read_ptr = current_buffer + sizeof(ion_linear_hash_bucket_t);
        for (int i = 0; i < num_records; i++) {
            key = read_ptr;
            if (ion_linear_hash_h0(key, lht) != ion_linear_hash_h1(key, lht)) {
                // This record belongs in the new bucket
                memcpy(insert_ptr_new, read_ptr, lht->record_total_size);
                new_bucket->records++;
                insert_ptr_new += lht->record_total_size;
                // If the new bucket is now full, write it out and initialize a new one.
                if (new_bucket->records == lht->records_per_bucket) {
                    err = ion_linear_hash_write_block(new_buffer, new_bucket->block, lht);
                    if (err_ok != err) {
                        // Clean up to try and reset the state
                        lht->total_blocks = starting_blocks;
                        // Exit. Note that this might be an in-consistent state but no records were lost.
                        return err;
                    }
                    ion_linear_hash_initialize_bucket(lht->total_blocks, new_buffer, LINEAR_HASH_BLOCK_SIZE,
                                                      new_bucket->block);
                    lht->total_blocks++;
                    insert_ptr_new = new_buffer + sizeof(ion_linear_hash_bucket_t);
                }

                // Remove the record
                memset(read_ptr, 0, lht->record_total_size);
                current_bucket->records--;
            } else {
                if (read_ptr != insert_ptr_current) {
                    memcpy(insert_ptr_current, read_ptr, lht->record_total_size);
                }
                insert_ptr_current += lht->record_total_size;
            }
            read_ptr += lht->record_total_size;
        }

        if (LINEAR_HASH_NO_OVERFLOW != current_bucket->overflow_block) {
            err = ion_linear_hash_write_block(current_buffer, current_bucket->block, lht);
            if (err_ok != err) {
                lht->total_blocks = starting_blocks;
                return err;
            }
            // Load the new bucket
            err = ion_linear_hash_read_block(current_bucket->overflow_block, lht, current_buffer);
            if (err_ok != err) {
                lht->total_blocks = starting_blocks;
                return err;
            }
            num_records = current_bucket->records;
            read_ptr = current_buffer + sizeof(ion_linear_hash_bucket_t);
        } else {
            completed = boolean_true;
        }
    }

    // Write out the changed and new blocks
    err = ion_linear_hash_write_block(current_buffer, current_bucket->block, lht);
    if (err_ok != err) {
        return err;
    }

    err = ion_linear_hash_write_block(new_buffer, new_bucket->block, lht);
    if (err_ok != err) {
        return err;
    }

    // Increment Bucket count and next split.
    lht->num_buckets++;
    if (lht->num_buckets == 2 * lht->initial_size + 1) {
        lht->initial_size = lht->initial_size * 2;
        lht->next_split = 0;
    }
    return err_ok;
}

ion_err_t
ion_linear_hash_increment_num_records(ion_linear_hash_table_t *table) {
    table->num_records++;
    if (table->num_records / table->num_buckets > table->split_threshold) {
        return ion_linear_hash_split(table);
    }
    return err_ok;
}

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

    ion_byte_t *buffer = linear_hash->block1;

    // Read the block from the file
    ion_err_t err = ion_linear_hash_read_block(block, linear_hash, buffer);
    if (err_ok != err) {
        return ION_STATUS_ERROR(err);
    }

    // Set the bucket pointer to the correct position
    ion_linear_hash_bucket_t *bucket;
    bucket = (ion_linear_hash_bucket_t *) buffer;

#if LINEAR_HASH_DEBUG > 0
    ion_linear_hash_debug_print_bucket(bucket);
#endif

    if (bucket->records == linear_hash->records_per_bucket) {
        ion_linear_hash_initialize_bucket(linear_hash->total_blocks, buffer, LINEAR_HASH_BLOCK_SIZE, bucket->block);
        ion_array_list_insert(idx, linear_hash->total_blocks, linear_hash->bucket_map);
        linear_hash->total_blocks++;
    }

    int record_size = linear_hash->super.record.key_size + linear_hash->super.record.value_size;
    int key_offset = sizeof(ion_linear_hash_bucket_t) + record_size * bucket->records;
    int value_offset = key_offset + linear_hash->super.record.value_size;
#if LINEAR_HASH_DEBUG > 0
    printf("Inserting key at offset %d, value at offset: %d\n", key_offset, value_offset);
#endif
    memcpy(buffer + key_offset, key, (size_t) linear_hash->super.record.key_size);
    memcpy(buffer + value_offset, value, (size_t) linear_hash->super.record.value_size);
    bucket->records++;

#if LINEAR_HASH_DEBUG > 0
    ion_key_t written_key;
    written_key = buffer + key_offset;
    printf("Written key ");
    ion_linear_hash_debug_print_key(written_key, linear_hash);
    printf("\n");
#endif
    ion_linear_hash_write_block(buffer, bucket->block, linear_hash);

    err = ion_linear_hash_increment_num_records(linear_hash);
    if (err_ok != err) {
        return ION_STATUS_ERROR(err);
    }

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
    // Read the first bucket
    int block = ion_array_list_get(idx, linear_hash->bucket_map);
    ion_byte_t *buffer;
    buffer = linear_hash->block1;
    ion_err_t read_bucket = ion_linear_hash_read_block(block, linear_hash, buffer);
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
    ion_byte_t *record_key;
    ion_byte_t *record_value;

    size_t offset = sizeof(ion_linear_hash_bucket_t);
    ion_boolean_t terminal = boolean_true;
    int i;
    while (terminal) {
#if LINEAR_HASH_DEBUG > 0
        printf("Reading records\n");
#endif
        for (i = 0; i < bucket->records; i = i + 1) {
            record_key = buffer + offset;
            record_value = record_key + linear_hash->super.record.key_size;
#if LINEAR_HASH_DEBUG > 0
            printf("Checking record %d with key ", i);
            ion_linear_hash_debug_print_key(record_key, linear_hash);
            printf("\n");
#endif

            if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
                printf("Value matches");
                memcpy(value, record_value, sizeof(int));
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
            ion_linear_hash_read_block(bucket->overflow_block, linear_hash, buffer);
            offset = sizeof(bucket);
        }
    }
#if LINEAR_HASH_DEBUG > 0
    printf("Done");
#endif
    return ION_STATUS_ERROR(err_item_not_found);
}

ion_status_t
ion_linear_hash_delete(
        ion_key_t key,
        ion_linear_hash_table_t *lht
) {
    ion_status_t status = ION_STATUS_INITIALIZE;

    // Find the bucket index for this key
    int idx = ion_linear_hash_h0((ion_byte_t *) key, lht);
    if (idx < lht->next_split) {
        idx = ion_linear_hash_h1((ion_byte_t *) key, lht);
    }

    // Get the file block for the first bucket
    // Load the bucket to split
    int block = ion_array_list_get(idx, lht->bucket_map);
    ion_byte_t *buffer = lht->block1;
    status.error = ion_linear_hash_read_block(block, lht, buffer);
    if (err_ok != status.error) {
        return status;
    }

    // The bucket header is the first bytes of the buffer
    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) buffer;

    // Terminal when there is no overflow buckets left
    ion_boolean_t terminal = boolean_false;

    // We modify the number of records so keep track of how many we started with.
    int record_count = bucket->records;

    // Read pointer point to the current record
    ion_byte_t *read_ptr;

    // Inserting pointer to shrink gaps
    ion_byte_t *insert_ptr;

    while (!terminal) {
        // Initialize for each bucket
        insert_ptr = read_ptr = buffer + sizeof(ion_linear_hash_bucket_t);

        for (int i = 0; i < record_count; i++) {

            if (0 == lht->super.compare(read_ptr, key, lht->super.record.key_size)) {
                // We should delete this record.
                status.error++;

                // Move the record into the insert spot
                if (insert_ptr != read_ptr) {
                    memcpy(insert_ptr, read_ptr, lht->record_total_size);
                    insert_ptr += lht->record_total_size;
                }
                // TODO: Not strictly necessary and probably effects performance?
                memset(read_ptr, 0, lht->record_total_size);
                bucket->records--;
                lht->num_records--;
            } else {
                if (insert_ptr == read_ptr) {
                    insert_ptr += lht->record_total_size;
                }
            }
            read_ptr += lht->record_total_size;
        }

        // TODO: We leave empty buckets in the chain for the moment, discuss about removing or changing how overflow works.

        status.error = ion_linear_hash_write_block(buffer, bucket->block, lht);
        if (err_ok == status.error) {
            return status;
        }

        if (LINEAR_HASH_NO_OVERFLOW == bucket->overflow_block) {
            terminal = boolean_true;
        } else {
            status.error = ion_linear_hash_read_block(bucket->overflow_block, lht, buffer);
            if (err_ok != status.error) {
                return status;
            }
        }
    }
    if (status.count == 0) {
        status.error = err_item_not_found;
    }
    return status;
}