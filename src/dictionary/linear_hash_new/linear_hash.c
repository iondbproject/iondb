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
#include <limits.h>
#include "../../file/kv_stdio_intercept.h"
#include "../dictionary.h"

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
ion_err_t
ion_linear_hash_initialize_new_bucket_for_idx(ion_byte_t *block, int idx, ion_linear_hash_table_t *lht) {
    // Preconditions
    if(INT_MAX == lht->total_buckets){
        return err_out_of_bounds;
    }

    memset(block, 0, LINEAR_HASH_BLOCK_SIZE);
    ion_linear_hash_bucket_t *bucket;
    bucket = (ion_linear_hash_bucket_t *) block;
    bucket->block = lht->next_block;
    lht->next_block++;
    bucket->records = 0;

    if (idx < lht->current_size) {
        int overflow = ion_array_list_get(idx, lht->bucket_map);
        bucket->overflow_block = overflow;
    } else {
        bucket->overflow_block = LINEAR_HASH_NO_OVERFLOW;
    }
    lht->total_buckets++;
    return ion_array_list_insert(idx, bucket->block, lht->bucket_map);
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
    long offset = ((long) block) * LINEAR_HASH_BLOCK_SIZE;
#if LINEAR_HASH_DEBUG_WRITE_BLOCK
    printf("Writing block %d to offset %lu\n", block, offset);
#endif
    if (0 != fseek(linear_hash->database, offset, SEEK_SET)) {
        return err_file_bad_seek;
    }

    if (1 != fwrite(bucket, LINEAR_HASH_BLOCK_SIZE, 1, linear_hash->database)) {
        return err_file_write_error;
    }
    return err_ok;
}

ion_err_t
ion_linear_hash_save_state(ion_linear_hash_table_t *table) {
    if (0 != fseek(table->state, 0, SEEK_SET)) {
        return err_file_bad_seek;
    }

    if (1 != fwrite(&(table->initial_size), sizeof(table->initial_size), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fwrite(&table->next_split, sizeof(table->next_split), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fwrite(&table->split_threshold, sizeof(table->split_threshold), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fwrite(&table->current_size, sizeof(table->current_size), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fwrite(&table->total_buckets, sizeof(table->total_buckets), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fwrite(&table->num_records, sizeof(table->num_records), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fwrite(&table->records_per_bucket, sizeof(table->records_per_bucket), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fwrite(&table->next_block, sizeof(table->next_block), 1, table->state)) {
        return err_file_read_error;
    }

    ion_err_t err = ion_array_list_save_to_file(table->state, table->bucket_map);
    if (err_ok != err) {
        return err;
    }
    return err_ok;
}

ion_err_t
ion_linear_hash_read_state(ion_linear_hash_table_t *table) {
    if (0 != fseek(table->state, 0, SEEK_SET)) {
        return err_file_bad_seek;
    }

    if (1 != fread(&(table->initial_size), sizeof(table->initial_size), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fread(&table->next_split, sizeof(table->next_split), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fread(&table->split_threshold, sizeof(table->split_threshold), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fread(&table->current_size, sizeof(table->current_size), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fread(&table->total_buckets, sizeof(table->total_buckets), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fread(&table->num_records, sizeof(table->num_records), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fread(&table->records_per_bucket, sizeof(table->records_per_bucket), 1, table->state)) {
        return err_file_read_error;
    }

    if (1 != fread(&table->next_block, sizeof(table->next_block), 1, table->state)) {
        return err_file_read_error;
    }

    ion_array_list_t *bucket_map = malloc(sizeof(ion_array_list_t));
    if (NULL == bucket_map) {
        return err_out_of_memory;
    }
    table->bucket_map = bucket_map;
    ion_err_t err = ion_array_list_init_from_file(table->state, bucket_map);
    if (err_ok != err) {
        return err;
    }
    return err_ok;
}

ion_err_t ion_linear_hash_close(ion_linear_hash_table_t *table) {
    ion_err_t err = err_ok;
#if LINEAR_HASH_DEBUG_CLOSE
    printf("Closing database\n");
#endif
    if (NULL != table->state) {
#if LINEAR_HASH_DEBUG_CLOSE
        printf("\tSaving the state\n");
#endif
        err = ion_linear_hash_save_state(table);
    }
#if LINEAR_HASH_DEBUG_CLOSE
    printf("\tClearing block cache\n");
#endif
    free(table->block1);
    table->block1 = NULL;
    free(table->block2);
    table->block2 = NULL;

#if LINEAR_HASH_DEBUG_CLOSE
    printf("\tClearing bucket map\n");
#endif
    if (NULL != table->bucket_map) {
#if LINEAR_HASH_DEBUG_CLOSE
        printf("\tDestroying bucket map\n");
#endif
        ion_array_list_destroy(table->bucket_map);
#if LINEAR_HASH_DEBUG_CLOSE
        printf("\tFreeing bucket map\n");
#endif
        free(table->bucket_map);
    }
#if LINEAR_HASH_DEBUG_CLOSE
        printf("\tClosing files\n");
#endif
    fclose(table->state);
    fclose(table->database);
    return err;
}

ion_err_t ion_linear_hash_initialize_new(ion_linear_hash_table_t *table) {
#if LINEAR_HASH_DEBUG_INIT
    printf("Initializing a new linear hash\n");
#endif
    ion_err_t err;

    // Initialize the bucket map
#if LINEAR_HASH_DEBUG_INIT
    printf("\tInitializing a a new bucket map\n");
#endif
    ion_array_list_t *bucket_map;
    bucket_map = malloc(sizeof(ion_array_list_t));
    if (NULL == bucket_map) {
#if LINEAR_HASH_DEBUG_INIT
        printf("\tUnable to initialize a new bucket map\n");
#endif
        return err_out_of_memory;
    }
    table->bucket_map = bucket_map;
#if LINEAR_HASH_DEBUG_INIT
    printf("\tInitializing bucket map with %d spots\n", table->initial_size * 2);
#endif
    err = ion_array_list_init(table->initial_size * 2, bucket_map);
    if (err_ok != err) {
        return err;
    }

    // Create initial blocks
#if LINEAR_HASH_DEBUG_INIT
    printf("\tInitializing %d blocks\n", table->initial_size);
#endif
    for (int i = 0; i < table->initial_size; i++) {
        ion_linear_hash_initialize_new_bucket_for_idx(table->block1, i, table);
        err = ion_linear_hash_write_block(table->block1, i, table);
        if (err_ok != err) {
            return err;
        }
        table->current_size++;
    }
#if LINEAR_HASH_DEBUG_INIT
    printf("\tCompleted initializing\n");
#endif
    return err_ok;
}


ion_err_t
ion_linear_hash_init(ion_dictionary_id_t id, ion_key_type_t key_type, ion_key_size_t key_size,
                     ion_value_size_t value_size, int initial_size, int split_threshold,
                     ion_linear_hash_table_t *linear_hash) {
#if LINEAR_HASH_DEBUG_INIT > 0
    printf("Initializing with id: %d\n", id);
#endif
    // Assign super dictionary info
    linear_hash->super.id = id;
    linear_hash->super.key_type = key_type;
    linear_hash->super.record.key_size = key_size;
    linear_hash->super.record.value_size = value_size;

    /* initialize linear_hash fields to default values, will be read from state*/
    linear_hash->split_threshold = split_threshold;
    linear_hash->initial_size = initial_size;
    linear_hash->current_size = 0;
    linear_hash->num_records = 0;
    linear_hash->next_split = 0;
    linear_hash->total_buckets = 0;
    linear_hash->next_block = 0;
    linear_hash->records_per_bucket =
            (LINEAR_HASH_BLOCK_SIZE - sizeof(ion_linear_hash_bucket_t)) / (key_size + value_size);
    linear_hash->record_total_size = key_size + value_size;

    // Initialize the bucket map to a null pointer as it is not created yet.
    // This prevents some errors trying to free it if closing early
    linear_hash->bucket_map = NULL;
    linear_hash->block1 = NULL;
    linear_hash->block2 = NULL;

    // Initialize the buffers
    linear_hash->block1 = malloc(LINEAR_HASH_BLOCK_SIZE);
    linear_hash->block2 = malloc(LINEAR_HASH_BLOCK_SIZE);
    if (linear_hash->block1 == NULL || linear_hash->block2 == NULL) {
        ion_linear_hash_close(linear_hash);
        return err_out_of_memory;
    }

    // initialize the file names.
    char data_filename[ION_MAX_FILENAME_LENGTH];
    dictionary_get_filename(linear_hash->super.id, "lhd", data_filename);
    char state_filename[ION_MAX_FILENAME_LENGTH];
    dictionary_get_filename(linear_hash->super.id, "lhs", state_filename);

    // TODO: Detect key type and assign
    if (sizeof(int) == linear_hash->super.record.key_size) {
        linear_hash->hash_key = ion_linear_hash_int_key_hash;
    } else {
        linear_hash->hash_key = ion_linear_hash_generic_key_hash;
    }

    // Try and restore the state:
    linear_hash->state = fopen(state_filename, "r+b");
    linear_hash->database = fopen(data_filename, "r+b");
    if (NULL == linear_hash->state || NULL == linear_hash->database) {
#if LINEAR_HASH_DEBUG_INIT > 0
        printf("Could find an existing database or state\n");
#endif
        // Initialize a new state
        fclose(linear_hash->state);
        fclose(linear_hash->database);

#if LINEAR_HASH_DEBUG_INIT > 0
        printf("Initializing new files\n");
#endif
        // Initialize new files
        linear_hash->state = fopen(state_filename, "w+");
        linear_hash->database = fopen(data_filename, "w+");

        if (NULL == linear_hash->state || NULL == linear_hash->database) {
#if LINEAR_HASH_DEBUG_INIT > 0
            if (NULL == linear_hash->state) {
                printf("Unable to initialize new state file, %s\n", state_filename);
            }

            if (NULL == linear_hash->database) {
                printf("Unable to initialize new data file, %s\n", data_filename);
            }
#endif
            ion_linear_hash_close(linear_hash);
#if LINEAR_HASH_DEBUG_INIT > 0
            printf("Closed gracefully\n");
#endif
            return err_file_open_error;
        }

#if LINEAR_HASH_DEBUG_INIT > 0
        printf("Created files %s, %s\n", state_filename, data_filename);
#endif

        // Initialize the new linear hash
        ion_err_t err = ion_linear_hash_initialize_new(linear_hash);
        if (err_ok != err) {
            ion_linear_hash_close(linear_hash);
            return err;
        }

    } else {
#if LINEAR_HASH_DEBUG_INIT > 0
        printf("Reading from existing state\n");
#endif

        // Read the existing state
        ion_err_t err = ion_linear_hash_read_state(linear_hash);
        if (err_ok != err) {
            ion_linear_hash_close(linear_hash);
            printf("Unable to read the existing state. Something has gone badly wrong.\n");
            return err;
        }
    }
    return err_ok;
}

//region Bucket Index Hashing

int ion_linear_hash_key_to_bucket_idx(ion_key_t key, ion_linear_hash_table_t *lht) {
    int hash = lht->hash_key(key, lht->super.record.key_size);
    int idx = ion_linear_hash_h0(hash, lht);
    if (idx < lht->next_split) {
        idx = ion_linear_hash_h1(hash, lht);
    }
    return idx;
}

int ion_linear_hash_h0(int hash, ion_linear_hash_table_t *linear_hash) {
    return hash & (linear_hash->initial_size - 1);
}

int ion_linear_hash_h1(int hash, ion_linear_hash_table_t *linear_hash) {
    return hash & ((2 * linear_hash->initial_size) - 1);
}

//endregion

//region Block writing

ion_err_t ion_linear_hash_read_block(int block, ion_linear_hash_table_t *linear_hash, ion_byte_t *buffer) {
    long offset = ((long) block) * LINEAR_HASH_BLOCK_SIZE;
#if LINEAR_HASH_DEBUG_READ_BLOCK
    printf("Reading block %d from offset %lu\n", block, offset);
#endif
    if (0 != fseek(linear_hash->database, offset, SEEK_SET)) {
        return err_file_bad_seek;
    }

    if (1 != fread(buffer, LINEAR_HASH_BLOCK_SIZE, 1, linear_hash->database)) {
        return err_file_read_error;
    }
    return err_ok;
}

//endregion

ion_err_t ion_linear_hash_split(ion_linear_hash_table_t *lht) {
#if LINEAR_HASH_DEBUG_SPLIT
    printf("\nStarting split\n");
#endif
    int current_split = lht->next_split;
    int new_idx = lht->current_size;
    int starting_blocks = lht->next_block;
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
    ion_linear_hash_initialize_new_bucket_for_idx(new_buffer, new_idx, lht);
    lht->current_size++;
    ion_linear_hash_bucket_t *new_bucket = (ion_linear_hash_bucket_t *) new_buffer;

    ion_boolean_t completed = boolean_false;

    // Initialize read pointers for swapping records
    ion_byte_t *insert_ptr_new = new_buffer + sizeof(ion_linear_hash_bucket_t);
    ion_byte_t *insert_ptr_current;
    ion_byte_t *read_ptr;
    ion_byte_t *key;
    int num_records;

    // Iterator

    while (!completed) {

        // Set read and write values for the current bucket
        num_records = current_bucket->records;
        insert_ptr_current = current_buffer + sizeof(ion_linear_hash_bucket_t);
        read_ptr = current_buffer + sizeof(ion_linear_hash_bucket_t);

        // Iterate through all records.
        for (int i = 0; i < num_records; ++i) {
            key = read_ptr;
            int hash = lht->hash_key(key, lht->super.record.key_size);

            if (ion_linear_hash_h0(hash, lht) != ion_linear_hash_h1(hash, lht)) {
                // This record belongs in the new bucket
                memcpy(insert_ptr_new, read_ptr, (size_t) lht->record_total_size);
                new_bucket->records++;
                insert_ptr_new += lht->record_total_size;

                // If the new bucket is now full, write it out and initialize a new one.
                if (new_bucket->records == lht->records_per_bucket) {
                    err = ion_linear_hash_write_block(new_buffer, new_bucket->block, lht);
                    if (err_ok != err) {
                        // Clean up to try and reset the state
                        lht->next_block = starting_blocks;
                        // Exit. Note that this might be an in-consistent state but no records were lost.
                        return err;
                    }
                    ion_linear_hash_initialize_new_bucket_for_idx(new_buffer, new_idx, lht);
                    insert_ptr_new = new_buffer + sizeof(ion_linear_hash_bucket_t);
                }

                // Remove the record
                memset(read_ptr, 0, (size_t) lht->record_total_size);
                current_bucket->records--;
            } else {
                if (read_ptr != insert_ptr_current) {
                    memcpy(insert_ptr_current, read_ptr, (size_t) lht->record_total_size);
                }
                insert_ptr_current += lht->record_total_size;
            }
            read_ptr += lht->record_total_size;
        }

        if (LINEAR_HASH_NO_OVERFLOW != current_bucket->overflow_block) {
            err = ion_linear_hash_write_block(current_buffer, current_bucket->block, lht);
            if (err_ok != err) {
                lht->next_block = starting_blocks;
                return err;
            }
            // Load the new bucket
            err = ion_linear_hash_read_block(current_bucket->overflow_block, lht, current_buffer);
            if (err_ok != err) {
                lht->next_block = starting_blocks;
                return err;
            }
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
    if (lht->current_size == 2 * lht->initial_size) {
        lht->initial_size = lht->initial_size * 2;
        lht->next_split = 0;
    } else {
        lht->next_split++;
    }
    return err_ok;
}

ion_err_t
ion_linear_hash_increment_num_records(ion_linear_hash_table_t *table) {
#if LINEAR_HASH_DEBUG_INCREMENT
    printf("Incrementing the number of records\n");
#endif
    table->num_records++;
    float load = (100.0f * table->num_records) / (table->current_size * table->records_per_bucket);
    ion_boolean_t above_threshold = (load > table->split_threshold);
#if LINEAR_HASH_DEBUG_INCREMENT
    printf("\tIncremented number of records to %d\n", table->num_records);
    printf("\tLoad is now: %d\n", (int) load);
#endif
    if (above_threshold) {
        return ion_linear_hash_split(table);
    }
    return err_ok;
}

ion_err_t
ion_linear_hash_insert_preconditions(ion_linear_hash_table_t *lht) {
    // Verify we aren't overflowing the maximum size
    if (UINT32_MAX == lht->num_records) {
        return err_out_of_bounds;
    }
    return err_ok;
}

ion_status_t ion_linear_hash_insert(
        ion_key_t key,
        ion_value_t value,
        ion_linear_hash_table_t *linear_hash
) {

#if LINEAR_HASH_DEBUG
    printf("\n********** Insert **********\n");
    printf("\nInserting with key: ");
    char str[linear_hash->super.record.key_size];
    strncpy(str, key, (size_t) linear_hash->super.record.key_size);
    printf("%s", str);
    printf("Inserting key: ");
    ion_linear_hash_debug_print_key(key, linear_hash);
    printf("\n");
#endif

    ion_err_t err = ion_linear_hash_insert_preconditions(linear_hash);

    if (err != err_ok) {
        return ION_STATUS_ERROR(err);
    }

    int idx = ion_linear_hash_key_to_bucket_idx(key, linear_hash);
    int block = ion_array_list_get(idx, linear_hash->bucket_map);

#if LINEAR_HASH_DEBUG
    printf("\n\tInto bucket index %d at block %d\n", idx, block);
#endif
    // Invalid block index (shouldn't happen)
    if (ARRAY_LIST_END_OF_LIST == block) {
        return ION_STATUS_ERROR(err_out_of_bounds);
    }

    ion_byte_t *buffer = linear_hash->block1;

    // Read the block from the file
    err = ion_linear_hash_read_block(block, linear_hash, buffer);
    if (err_ok != err) {
#if LINEAR_HASH_DEBUG_ERRORS
        printf("Insert failed to read block %d with error %d. Current blocks: [0..%d)\n", block, err,
               linear_hash->next_block);
#endif
        return ION_STATUS_ERROR(err);
    }

    // Set the bucket pointer to the correct position
    ion_linear_hash_bucket_t *bucket;
    bucket = (ion_linear_hash_bucket_t *) buffer;

#if LINEAR_HASH_DEBUG > 0
    ion_linear_hash_debug_print_bucket(bucket);
#endif

    if (bucket->records == linear_hash->records_per_bucket) {
#if LINEAR_HASH_DEBUG > 0
        printf("\tBucket was full %d, initializing a new bucket\n", bucket->records);
#endif
        err = ion_linear_hash_initialize_new_bucket_for_idx(buffer, idx, linear_hash);
        if (err_ok != err) {
#if LINEAR_HASH_DEBUG_ERRORS
            printf("Insert failed to initialize new idx %d with error %d\n", block, err);
#endif
            return ION_STATUS_ERROR(err);
        }
    }
#if LINEAR_HASH_DEBUG > 0
    printf("\tInserting into bucket with current size %d and block %d\n", bucket->records, bucket->block);
#endif
    size_t key_offset = sizeof(ion_linear_hash_bucket_t) + (linear_hash->record_total_size * bucket->records);
    size_t value_offset = key_offset + linear_hash->super.record.key_size;
    ion_byte_t *key_loc = buffer + key_offset;
    ion_byte_t *value_loc = buffer + value_offset;

#if LINEAR_HASH_DEBUG > 0
    printf("Inserting key at offset %d, value at offset: %d\n", key_offset, value_offset);
#endif
    memcpy(key_loc, key, (size_t) linear_hash->super.record.key_size);
    memcpy(value_loc, value, (size_t) linear_hash->super.record.value_size);
    bucket->records++;

#if LINEAR_HASH_DEBUG > 0
    ion_key_t written_key;
    written_key = buffer + key_offset;
    printf("Written key ");
    ion_linear_hash_debug_print_key(written_key, linear_hash);
    printf("\n");
#endif
    err = ion_linear_hash_write_block(buffer, bucket->block, linear_hash);

    if (err_ok != err) {
#if LINEAR_HASH_DEBUG_ERRORS
        printf("Insert failed to write block %d with error %d\n", block, err);
#endif
        return ION_STATUS_ERROR(err);
    }

    err = ion_linear_hash_increment_num_records(linear_hash);
    if (err_ok != err) {
#if LINEAR_HASH_DEBUG_ERRORS
        printf("Insert failed with error %d\n", err);
#endif
        return ION_STATUS_ERROR(err);
    }

    return ION_STATUS_OK(1);
}

ion_status_t
ion_linear_hash_get(ion_key_t key, ion_value_t value, ion_linear_hash_table_t *linear_hash) {
#if LINEAR_HASH_DEBUG > 0
    printf("\n********** Get **********\n");
    printf("Getting key: ");
    ion_linear_hash_debug_print_key(key, linear_hash);
    printf("\n");
#endif

    // Calculate the block index
    int idx = ion_linear_hash_key_to_bucket_idx(key, linear_hash);

    // Read the first bucket
    int block = ion_array_list_get(idx, linear_hash->bucket_map);
    ion_byte_t *buffer;
    buffer = linear_hash->block1;
    ion_err_t read_bucket = ion_linear_hash_read_block(block, linear_hash, buffer);
    if (err_ok != read_bucket) {
        return ION_STATUS_ERROR(read_bucket);
    }

    ion_linear_hash_bucket_t *bucket;
    bucket = (ion_linear_hash_bucket_t *) buffer;

    ion_byte_t *record_key;
    ion_byte_t *record_value;
    ion_boolean_t terminal = boolean_true;
    int i;
    while (terminal) {
        record_key = buffer + sizeof(ion_linear_hash_bucket_t);
        for (i = 0; i < bucket->records; ++i) {
            if (linear_hash->super.compare(record_key, key, linear_hash->super.record.key_size) == 0) {
                record_value = record_key + linear_hash->super.record.key_size;
                memcpy(value, record_value, (size_t) linear_hash->super.record.value_size);
                return ION_STATUS_OK(1);
            }
            record_key += linear_hash->record_total_size;
        }
        if (LINEAR_HASH_NO_OVERFLOW == bucket->overflow_block) {
            terminal = boolean_false;
        } else {
            ion_linear_hash_read_block(bucket->overflow_block, linear_hash, buffer);
        }
    }
    return ION_STATUS_ERROR(err_item_not_found);
}

ion_status_t
ion_linear_hash_delete(
        ion_key_t key,
        ion_linear_hash_table_t *lht
) {
#if LINEAR_HASH_DEBUG
    printf("\nDeleting with key: ");
    char str[lht->super.record.key_size];
    strncpy(str, key, (size_t) lht->super.record.key_size);
    printf("%s\n", str);
#endif
    ion_status_t status = ION_STATUS_INITIALIZE;

    // Find the bucket index for this key
    int idx = ion_linear_hash_key_to_bucket_idx(key, lht);


    // Get the file block for the first bucket
    // Load the bucket to split
    int block = ion_array_list_get(idx, lht->bucket_map);

#if LINEAR_HASH_DEBUG
    printf("\tFrom bucket %d at block %d\n", idx, block);
#endif

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
    int record_count;

    // Read pointer point to the current record
    ion_byte_t *read_ptr;

    // Inserting pointer to shrink gaps
    ion_byte_t *insert_ptr;

    ion_boolean_t dirty;

    while (!terminal) {
        // Initialize for each bucket
        dirty = boolean_false;
        record_count = bucket->records;
        insert_ptr = buffer + sizeof(ion_linear_hash_bucket_t);
        read_ptr = buffer + sizeof(ion_linear_hash_bucket_t);

        for (int i = 0; i < record_count; i++) {
#if LINEAR_HASH_DEBUG
            char strkey[lht->super.record.key_size];
            strncpy(strkey, read_ptr, (size_t) lht->super.record.key_size);
            printf("\tComparing to key %s\n", strkey);
#endif
            if (0 == lht->super.compare(read_ptr, key, lht->super.record.key_size)) {
                // We should delete this record.
#if LINEAR_HASH_DEBUG
                printf("\t\tDeleting this record\n");
#endif
                status.count++;
                memset(read_ptr, 0, (size_t) lht->record_total_size);
                bucket->records--;
                lht->num_records--;
                dirty = boolean_true;
                // Don't increment the insert_ptr, we will insert to this location
            } else {
                if (insert_ptr != read_ptr) {
#if LINEAR_HASH_DEBUG
                    printf("\t\tShifting the record up\n");
#endif
                    // Shift this record to the insert spot
                    memcpy(insert_ptr, read_ptr, (size_t) lht->record_total_size);
                }
                insert_ptr += lht->record_total_size;
            }
            read_ptr += lht->record_total_size;
        }

        // TODO: We leave empty buckets in the chain for the moment, discuss about removing or changing how overflow works.

        if(boolean_true == dirty) {
            status.error = ion_linear_hash_write_block(buffer, bucket->block, lht);
            if (err_ok != status.error) {
                return status;
            }
        }

        if (LINEAR_HASH_NO_OVERFLOW == bucket->overflow_block) {
#if LINEAR_HASH_DEBUG
            printf("\tNo more overflow buckets\n");
#endif
            terminal = boolean_true;
        } else {
#if LINEAR_HASH_DEBUG
            printf("\tReading overflow bucket from block %d from block %d\n", bucket->overflow_block, bucket->block);
#endif
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

ion_status_t
ion_linear_hash_update(ion_key_t key, ion_value_t value, ion_linear_hash_table_t *lht) {

    ion_status_t status = ION_STATUS_INITIALIZE;

    // Find the bucket index for this key
    int idx = ion_linear_hash_key_to_bucket_idx(key, lht);

    // The first file block to use
    int block = ion_array_list_get(idx, lht->bucket_map);

    ion_byte_t *buffer = lht->block1;

    // Bucket header in the block
    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) buffer;

    // If there is nothing to update we can save writes.
    ion_boolean_t block_is_dirty;

    // Read ptr for the key in each record
    ion_byte_t *read_ptr;

    // True once all blocks have been read.
    ion_boolean_t terminal = boolean_false;

    while (!terminal) {

        // Read the current bucket block (updated at the end of the loop to the new overflow)
        status.error = ion_linear_hash_read_block(block, lht, buffer);
        if (err_ok != status.error) {
            return status;
        }

        // Reset bucket block variables.
        block_is_dirty = boolean_false;
        read_ptr = buffer + sizeof(ion_linear_hash_bucket_t); // Start of records.

        for (int i = 0; i < bucket->records; i++) {
            if (0 == lht->super.compare(read_ptr, key, lht->super.record.key_size)) {
                // Key matches copy value and increment the count
                status.count++;
                memcpy(read_ptr + lht->super.record.key_size, value, (size_t) lht->super.record.value_size);
                block_is_dirty = boolean_true;
            }
            read_ptr += lht->record_total_size;
        }

        // Only write the block if it was changed.
        if (block_is_dirty) {
            status.error = ion_linear_hash_write_block(buffer, bucket->block, lht);
            if (err_ok != status.error) {
                return status;
            }
        }

        if (LINEAR_HASH_NO_OVERFLOW != bucket->overflow_block) {
            block = bucket->overflow_block;
        } else {
            // All done!
            terminal = boolean_true;
        }
    }

    if (0 == status.count) { // Perform an upsert
        return ion_linear_hash_insert(key, value, lht);
    }
    return status;
}

int ion_linear_hash_int_key_hash(ion_key_t key, ion_key_size_t key_size) {
    return *(int *) key;
}

int
ion_linear_hash_generic_key_hash(ion_key_t key, ion_key_size_t key_size) {
    int i;
    ion_byte_t *key_bytes = (ion_byte_t *) key;
    int key_bytes_as_int = 0;
    static int coefficients[] = {3, 5, 7, 11, 13, 17, 19};

    for (i = 0; i < key_size - 1; i++) {
        key_bytes_as_int += *(key_bytes + i) * coefficients[i + 1] - *(key_bytes + i) * coefficients[i];
    }

    return key_bytes_as_int;
}
