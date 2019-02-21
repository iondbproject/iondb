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
        ion_linear_hash_bucket_header_t *bucket
) {
    printf("Bucket %d: record count: %d, overflow: %d, size: %lu\n", bucket->block, bucket->records,
           bucket->overflow_block, sizeof(ion_linear_hash_bucket_header_t));
}

#endif

ion_err_t
ion_linear_hash_close(
        ion_linear_hash_table_t *table
) {
    ion_err_t err = err_ok;

    LH_DEBUG_CLOSE("Closing database\n");
    if (NULL != table->state) {
        LH_DEBUG_CLOSE("\tSaving the state\n");
        err = ion_linear_hash_save_state(table);
    }

    LH_DEBUG_CLOSE("\tClearing block cache\n");
    free(table->buffer1);
    table->buffer1 = NULL;
    free(table->buffer2);
    table->buffer2 = NULL;

    LH_DEBUG_CLOSE("\tClosing files\n");
    fclose(table->state);
    fclose(table->database);
    fclose(table->overflow);
    table->state = NULL;
    table->database = NULL;
    table->overflow = NULL;
    return err;
}

ion_err_t
ion_linear_hash_initialize_new(
        ion_linear_hash_table_t *table
) {
#if LINEAR_HASH_DEBUG_INIT
    printf("Initializing a new linear hash\n");
#endif

    ion_err_t err;

    /* Create initial blocks */
#if LINEAR_HASH_DEBUG_INIT
    printf("\tInitializing %d blocks\n", table->initial_size);
#endif

    for (int i = 0; i < table->initial_size; i++) {
        ion_linear_hash_initialize_new_bucket_for_idx(table->buffer1, (ion_linear_hash_bucket_index) i, table, DATA);
        err = ion_linear_hash_write_buffer(table, table->buffer1);

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
ion_linear_hash_init(
        ion_dictionary_id_t id,
        ion_key_type_t key_type,
        ion_key_size_t key_size,
        ion_value_size_t value_size,
        int initial_size,
        int split_threshold,
        ion_linear_hash_table_t *linear_hash
) {
#if LINEAR_HASH_DEBUG_INIT > 0
    printf("Initializing with id: %d\n", id);
#endif
    /* Assign super dictionary info */
    linear_hash->super.id = id;
    linear_hash->super.key_type = key_type;
    linear_hash->super.record.key_size = key_size;
    linear_hash->super.record.value_size = value_size;

    /* initialize linear_hash fields to default values, will be read from state*/
    linear_hash->split_threshold = split_threshold;
    linear_hash->initial_size = (ion_linear_hash_bucket_index) initial_size;
    linear_hash->current_size = 0;
    linear_hash->num_records = 0;
    linear_hash->next_split = 0;
    linear_hash->total_buckets = 0;
    linear_hash->next_block = 0;
    linear_hash->records_per_bucket =
            (LINEAR_HASH_BLOCK_SIZE - sizeof(ion_linear_hash_bucket_header_t)) / (key_size + value_size);
    linear_hash->record_total_size = key_size + value_size;

    /* Initialize the bucket map to a null pointer as it is not created yet. */
    /* This prevents some errors trying to free it if closing early */
    linear_hash->buffer1 = NULL;
    linear_hash->buffer2 = NULL;

    linear_hash->ion_linear_hash_block_writes = 0;
    linear_hash->ion_linear_hash_block_reads = 0;

    /* Initialize the buffers */
    linear_hash->buffer1 = malloc(sizeof(ion_linear_hash_buffer_t));
    linear_hash->buffer2 = malloc(sizeof(ion_linear_hash_buffer_t));

    if ((linear_hash->buffer1 == NULL) || (linear_hash->buffer2 == NULL)) {
#if LINEAR_HASH_DEBUG_INIT
        printf("Unable to allocate memory for main buffers");
#endif
        ion_linear_hash_close(linear_hash);
        return err_out_of_memory;
    }

    linear_hash->buffer1->type = EMPTY;
    linear_hash->buffer2->type = EMPTY;

    /* initialize the file names. */
    char data_filename[ION_MAX_FILENAME_LENGTH];
    dictionary_get_filename(linear_hash->super.id, "lhd", data_filename);
    char state_filename[ION_MAX_FILENAME_LENGTH];
    dictionary_get_filename(linear_hash->super.id, "lhs", state_filename);
    char overflow_filename[ION_MAX_FILENAME_LENGTH];
    dictionary_get_filename(linear_hash->super.id, "lho", overflow_filename);

    // Default hash function
    linear_hash->hash_key_function = ion_linear_hash_sdbm_hash;

    /* Try and restore the state: */
    linear_hash->state = fopen(state_filename, "r+b");
    linear_hash->database = fopen(data_filename, "r+b");
    linear_hash->overflow = fopen(overflow_filename, "r+b");

    if ((NULL == linear_hash->state) || (NULL == linear_hash->database) || (NULL == linear_hash->overflow)) {
#if LINEAR_HASH_DEBUG_INIT > 0
        printf("Could not find an existing database or state\n");
#endif
        /* Initialize a new state */
        fclose(linear_hash->state);
        fclose(linear_hash->database);
        fclose(linear_hash->overflow);

#if LINEAR_HASH_DEBUG_INIT > 0
        printf("Initializing new files\n");
#endif
        /* Initialize new files */
        linear_hash->state = fopen(state_filename, "w+");
        linear_hash->database = fopen(data_filename, "w+");
        linear_hash->overflow = fopen(overflow_filename, "w+");

        if ((NULL == linear_hash->state) || (NULL == linear_hash->database)) {
#if LINEAR_HASH_DEBUG_INIT > 0

            if (NULL == linear_hash->state) {
                printf("Unable to initialize new state file, %s\n", state_filename);
            }

            if (NULL == linear_hash->database) {
                printf("Unable to initialize new data file, %s\n", data_filename);
            }

            if (NULL == linear_hash->overflow) {
                printf("Unable to initialize new overflow file, %s\n", data_filename);
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

        /* Initialize the new linear hash */
        ion_err_t err = ion_linear_hash_initialize_new(linear_hash);

        if (err_ok != err) {
            ion_linear_hash_close(linear_hash);
            return err;
        }
    } else {
#if LINEAR_HASH_DEBUG_INIT > 0
        printf("Reading from existing state\n");
#endif

        /* Read the existing state */
        ion_err_t err = ion_linear_hash_read_state(linear_hash);

        if (err_ok != err) {
            ion_linear_hash_close(linear_hash);
            printf("Unable to read the existing state. Something has gone badly wrong.\n");
            return err;
        }
    }

    return err_ok;
}

//endregion

ion_err_t
ion_linear_hash_split(
        ion_linear_hash_table_t *lht
) {
    LH_SPLIT_DEBUG("\nStarting split\n", NULL);

    ion_linear_hash_bucket_index current_split = lht->next_split;
    ion_linear_hash_bucket_index new_idx = (ion_linear_hash_bucket_index) lht->current_size;
    ion_linear_hash_block_index_t starting_blocks = lht->next_block;
    /* Load the bucket to split */
    ion_linear_hash_buffer_t *splitting_buffer = ion_linear_hash_read_data_block(lht, lht->buffer1, current_split);

    LH_SPLIT_DEBUG("\nNext block %d\n", lht->next_block);

    ion_err_t err = splitting_buffer->err;
    ion_linear_hash_bucket_header_t *splitting_bucket = &splitting_buffer->block.bucket.header;

    if (err_ok != err) {
        return err;
    }

    ion_linear_hash_buffer_t *new_buffer = lht->buffer2;

    ion_linear_hash_initialize_new_bucket_for_idx(new_buffer, new_idx, lht, DATA);
    lht->current_size++;

    ion_linear_hash_bucket_header_t *new_bucket = &new_buffer->block.bucket.header;

    ion_boolean_t completed = boolean_false;

    /* Initialize read pointers for swapping records */
    ion_byte_t *insert_ptr_new = new_buffer->block.bucket.data;
    ion_byte_t *insert_ptr_current;
    ion_byte_t *read_ptr;
    ion_byte_t *key;
    int num_records;

    /* Iterator */

    while (!completed) {
        /* Set read and write values for the current bucket */
        num_records = splitting_bucket->records;
        insert_ptr_current = splitting_buffer->block.bucket.data;
        read_ptr = splitting_buffer->block.bucket.data;

        /* Iterate through all records. */
        for (int i = 0; i < num_records; ++i) {
            key = read_ptr;

            ion_linear_hash_key_hash hash = lht->hash_key_function(key, lht->super.record.key_size);

            if (ion_linear_hash_h0(hash, lht) != ion_linear_hash_h1(hash, lht)) {
                /* This record belongs in the new bucket */
                memcpy(insert_ptr_new, read_ptr, (size_t) lht->record_total_size);
                new_bucket->records++;
                new_buffer->dirty = boolean_true;
                insert_ptr_new += lht->record_total_size;

                /* If the new bucket is now full, write it out and initialize a new one. */
                if (new_bucket->records == lht->records_per_bucket) {
                    new_bucket->overflow_block = lht->next_block;
                    err = ion_linear_hash_write_buffer(lht, new_buffer);

                    if (err_ok != err) {
                        /* Clean up to try and reset the state */
                        lht->next_block = starting_blocks;
                        /* Exit. Note that this might be an in-consistent state but no records were lost. */
                        return err;
                    }

                    err = ion_linear_hash_initialize_new_bucket_for_idx(new_buffer, new_idx, lht, OVERFLOW);
                    insert_ptr_new = new_buffer->block.bucket.data;
                }

                /* Remove the record */
                memset(read_ptr, 0, (size_t) lht->record_total_size);
                splitting_buffer->dirty = boolean_true;
                splitting_bucket->records--;
            } else {
                if (read_ptr != insert_ptr_current) {
                    memcpy(insert_ptr_current, read_ptr, (size_t) lht->record_total_size);
                }

                insert_ptr_current += lht->record_total_size;
            }

            read_ptr += lht->record_total_size;
        }

        if (LINEAR_HASH_NO_OVERFLOW != splitting_bucket->overflow_block) {
            err = ion_linear_hash_write_buffer(lht, splitting_buffer);

            if (err_ok != err) {
                lht->next_block = starting_blocks;
                return err;
            }

            /* Load the new bucket */
            splitting_buffer = ion_linear_hash_read_overflow_block(lht, splitting_buffer,
                                                                   splitting_bucket->overflow_block);
            err = splitting_buffer->err;
            if (err_ok != err) {
                lht->next_block = starting_blocks;
                return err;
            }
        } else {
            completed = boolean_true;
        }
    }

    /* Write out the changed and new blocks */
    err = ion_linear_hash_write_buffer(lht, splitting_buffer);

    if (err_ok != err) {
        return err;
    }

    err = ion_linear_hash_write_buffer(lht, new_buffer);

    if (err_ok != err) {
        return err;
    }

    /* Increment Bucket count and next split. */
    if (lht->current_size == 2 * lht->initial_size) {
        lht->initial_size = lht->initial_size * 2;
        lht->next_split = 0;
    } else {
        lht->next_split++;
    }

    return err_ok;
}

ion_err_t
ion_linear_hash_increment_num_records(
        ion_linear_hash_table_t *table
) {
#if LINEAR_HASH_DEBUG_INCREMENT
    printf("Incrementing the number of records\n");
#endif
    table->num_records++;

    ion_boolean_t above_threshold = ion_linear_hash_check_above_threshold(table);

    if (boolean_true == above_threshold) {
#if LINEAR_HASH_DEBUG_INCREMENT
        printf("\tTriggering Split\n");
#endif
        return ion_linear_hash_split(table);
    }

    return err_ok;
}

ion_status_t
ion_linear_hash_insert(
        ion_key_t key,
        ion_value_t value,
        ion_linear_hash_table_t *linear_hash
) {
    LH_INSERT_DEBUG("\n********** Insert **********\n\nInserting with key: ", NULL);
#if LINEAR_HASH_DEBUG
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

    ion_linear_hash_bucket_index idx = ion_linear_hash_key_to_bucket_idx(key, linear_hash);

    LH_INSERT_DEBUG("\n\tInto bucket index %d\n", idx);

    ion_linear_hash_buffer_t *buffer = ion_linear_hash_read_data_block(linear_hash, linear_hash->buffer1, idx);

    if (err_ok != buffer->err) {
        LH_ERROR("Insert failed to read block %d with error %d. Current blocks: [0..%d)\n", buffer->block_index,
                 buffer->err,
                 linear_hash->next_block);
        return ION_STATUS_ERROR(err);
    }

    /* Set the bucket pointer to the correct position */
    ion_linear_hash_bucket_header_t *bucket = &buffer->block.bucket.header;

#if LINEAR_HASH_DEBUG
    ion_linear_hash_debug_print_bucket(bucket);
#endif
    ion_boolean_t terminal = boolean_true;

    // Iterate through all buckets until we find a non-full bucket or read the end of the chain.
    while (terminal) {
        if (buffer->block.bucket.header.records == linear_hash->records_per_bucket) {
            if (LINEAR_HASH_NO_OVERFLOW == buffer->block.bucket.header.overflow_block) {
                LH_INSERT_DEBUG("\tAll bucket were full %d, initializing a new bucket\n", bucket->records);
                ion_linear_hash_buffer_t *overflow_buffer = linear_hash->buffer2;
                err = ion_linear_hash_initialize_new_bucket_for_idx(overflow_buffer, idx, linear_hash, OVERFLOW);
                if (err_ok != err) {
                    LH_ERROR("\tUnable to initialize a new overflow bucket for index %d at block %lu\n", idx,
                             linear_hash->buffer2->block_index);
                    return ION_STATUS_ERROR(err);
                }

                buffer->block.bucket.header.overflow_block = overflow_buffer->block_index;
                buffer->dirty = boolean_true;
                err = ion_linear_hash_write_buffer(linear_hash, buffer);

                if (err_ok != buffer->err) {
                    LH_ERROR("\tUnable to write the block %lu\n", idx, buffer->block_index);
                    return ION_STATUS_ERROR(err);
                }
                buffer = overflow_buffer;
                bucket = &buffer->block.bucket.header;
            } else {
                LH_INSERT_DEBUG("\tLoading the next overflow bucket\n");
                ion_linear_hash_block_index_t overflow = buffer->block.bucket.header.overflow_block;
                buffer = ion_linear_hash_read_overflow_block(linear_hash, buffer, overflow);
                if (err_ok != buffer->err) {
                    LH_ERROR("\tUnable to read overflow block %lu\n", overflow);
                    return ION_STATUS_ERROR(err);
                }
            }
        } else {
            terminal = boolean_false;
        }
    }

    // We should now have a bucket with a valid place to insert
    // Calculate offsets
    size_t key_offset = (size_t) (linear_hash->record_total_size * buffer->block.bucket.header.records);
    size_t value_offset = key_offset + linear_hash->super.record.key_size;
    ion_byte_t *key_loc = buffer->block.bucket.data + key_offset;
    ion_byte_t *value_loc = buffer->block.bucket.data + value_offset;

    LH_INSERT_DEBUG("\tInserting key at offset %u, value at offset: %d\n", key_offset, value_offset);
    memcpy(key_loc, key, (size_t) linear_hash->super.record.key_size);
    memcpy(value_loc, value, (size_t) linear_hash->super.record.value_size);
    buffer->block.bucket.header.records++;
    buffer->dirty = boolean_true;

#if LINEAR_HASH_DEBUG > 0
    ion_key_t written_key;

    written_key = buffer->block.bucket.data + key_offset;
    printf("\tWritten key ");
    ion_linear_hash_debug_print_key(written_key, linear_hash);
    printf("\n");
#endif
    err = ion_linear_hash_write_buffer(linear_hash, buffer);

    if (err_ok != err) {
        LH_ERROR("\tInsert failed to write block %d with error %d\n", buffer->block_index, err);
        return ION_STATUS_ERROR(err);
    }

    err = ion_linear_hash_increment_num_records(linear_hash);

    if (err_ok != err) {
        LH_ERROR("Insert failed with error %d\n", err);
        return ION_STATUS_ERROR(err);
    }

    LH_INSERT_DEBUG("Completed insert\n", NULL);
    return ION_STATUS_OK(1);
}

ion_status_t
ion_linear_hash_get(
        ion_key_t key,
        ion_value_t value,
        ion_linear_hash_table_t *linear_hash
) {
#if LINEAR_HASH_DEBUG > 0
    printf("\n********** Get **********\n");
    printf("Getting key: ");
    ion_linear_hash_debug_print_key(key, linear_hash);
    printf("\n");
#endif

    // Calculate the top level bucket index
    ion_linear_hash_bucket_index idx = ion_linear_hash_key_to_bucket_idx(key, linear_hash);
    ion_linear_hash_buffer_t *buffer = ion_linear_hash_read_data_block(linear_hash, linear_hash->buffer1, idx);

    if (err_ok != buffer->err) {
        return ION_STATUS_ERROR(buffer->err);
    }

    ion_linear_hash_bucket_header_t *bucket;

    bucket = &buffer->block.bucket.header;

    ion_byte_t *record_key;
    ion_byte_t *record_value;
    ion_boolean_t terminal = boolean_true;
    int i;

    while (terminal) {
        record_key = buffer->block.bucket.data;

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
            buffer = ion_linear_hash_read_overflow_block(linear_hash, buffer, bucket->overflow_block);
            if (err_ok != buffer->err) {
                return ION_STATUS_ERROR(buffer->err);
            }
            bucket = &buffer->block.bucket.header;
        }
    }
    return ION_STATUS_ERROR(err_item_not_found);
}

ion_status_t
ion_linear_hash_delete(
        ion_key_t key,
        ion_linear_hash_table_t *lht
) {
#if LINEAR_HASH_DEBUG_DELETE
    printf("\nDeleting with key: ");

    char str[lht->super.record.key_size];

    strncpy(str, key, (size_t) lht->super.record.key_size);
    printf("%s\n", str);
#endif

    ion_status_t status = ION_STATUS_INITIALIZE;

    /* Find the bucket index for this key */
    ion_linear_hash_bucket_index idx = ion_linear_hash_key_to_bucket_idx(key, lht);
    ion_linear_hash_buffer_t *buffer = ion_linear_hash_read_data_block(lht, lht->buffer1, idx);

//    LH_DELETE_DEBUG("\tFrom bucket %d at block %d\n", idx, buffer->block_index);

    status.error = buffer->err;

    if (err_ok != status.error) {
        return status;
    }

    /* The bucket header is the first bytes of the buffer */
    ion_linear_hash_bucket_header_t *bucket = &buffer->block.bucket.header;

    /* Terminal when there is no overflow buckets left */
    ion_boolean_t terminal = boolean_false;

    /* We modify the number of records so keep track of how many we started with. */
    int record_count;

    /* Read pointer point to the current record */
    ion_byte_t *read_ptr;

    /* Inserting pointer to shrink gaps */
    ion_byte_t *insert_ptr;

    while (!terminal) {
        /* Initialize for each bucket */
        record_count = bucket->records;
        insert_ptr = buffer->block.bucket.data;
        read_ptr = buffer->block.bucket.data;

        for (int i = 0; i < record_count; i++) {
#if LINEAR_HASH_DEBUG_DELETE
            char strkey[lht->super.record.key_size];

            strncpy(strkey, read_ptr, (size_t) lht->super.record.key_size);
            printf("\tComparing to key %s\n", strkey);
#endif
            if (0 == lht->super.compare(read_ptr, key, lht->super.record.key_size)) {
                /* We should delete this record. */
                LH_DELETE_DEBUG("\t\tDeleting this record\n", NULL);
                status.count++;
                memset(read_ptr, 0, (size_t) lht->record_total_size);
                bucket->records--;
                lht->num_records--;
                buffer->dirty = boolean_true;
                /* Don't increment the insert_ptr, we will insert to this location */
            } else {
                if (insert_ptr != read_ptr) {
                    LH_DELETE_DEBUG("\t\tShifting the record up\n", NULL);
                    /* Shift this record to the insert spot */
                    memcpy(insert_ptr, read_ptr, (size_t) lht->record_total_size);
                }

                insert_ptr += lht->record_total_size;
            }
            read_ptr += lht->record_total_size;
        }

        /* TODO: We leave empty buckets in the chain for the moment, discuss about removing or changing how overflow works. */

        if (boolean_true == buffer->dirty) {
            status.error = ion_linear_hash_write_buffer(lht, buffer);

            if (err_ok != status.error) {
                return status;
            }
        }

        if (LINEAR_HASH_NO_OVERFLOW == bucket->overflow_block) {
            LH_DELETE_DEBUG("\tNo more overflow buckets\n", NULL);
            terminal = boolean_true;
        } else {
            LH_DELETE_DEBUG("\tReading overflow bucket from block %d from block %d\n", bucket->overflow_block,
                            buffer->block_index);
            buffer = ion_linear_hash_read_overflow_block(lht, buffer, bucket->overflow_block);
            status.error = buffer->err;

            if (err_ok != status.error) {
                return status;
            }
            bucket = &buffer->block.bucket.header;
        }
    }

    if (status.count == 0) {
        status.error = err_item_not_found;
    }
    return status;
}

ion_status_t
ion_linear_hash_update(
        ion_key_t key,
        ion_value_t value,
        ion_linear_hash_table_t *lht
) {
    ion_status_t status = ION_STATUS_INITIALIZE;

    /* Find the bucket index for this key */
    ion_linear_hash_bucket_index idx = ion_linear_hash_key_to_bucket_idx(key, lht);

    /* The first file block to use */
    ion_linear_hash_buffer_t *buffer_t = ion_linear_hash_read_data_block(lht, lht->buffer1, idx);

    /* Bucket header in the block */
    ion_linear_hash_bucket_header_t *bucket = &buffer_t->block.bucket.header;

    /* Read ptr for the key in each record */
    ion_byte_t *read_ptr;

    /* True once all blocks have been read. */
    ion_boolean_t terminal = boolean_false;

    while (!terminal) {
        /* Reset bucket block variables. */
        read_ptr = buffer_t->block.bucket.data;/* Start of records. */

        for (int i = 0; i < bucket->records; i++) {
            if (0 == lht->super.compare(read_ptr, key, lht->super.record.key_size)) {
                /* Key matches copy value and increment the count */
                status.count++;
                memcpy(read_ptr + lht->super.record.key_size, value, (size_t) lht->super.record.value_size);
                buffer_t->dirty = boolean_true;
            }
            read_ptr += lht->record_total_size;
        }

        /* Only write the block if it was changed. */
        if (buffer_t->dirty) {
            status.error = ion_linear_hash_write_buffer(lht, buffer_t);

            if (err_ok != status.error) {
                return status;
            }
        }

        // Read next bucket if possible.
        if (LINEAR_HASH_NO_OVERFLOW != bucket->overflow_block) {
            ion_linear_hash_read_overflow_block(lht, buffer_t, bucket->overflow_block);
            if (err_ok != buffer_t->err) {
                status.error = buffer_t->err;
                return status;
            }
            bucket = &buffer_t->block.bucket.header;
        } else {
            /* All done! */
            terminal = boolean_true;
        }
    }

    if (0 == status.count) {
        /* Perform an upsert */
        return ion_linear_hash_insert(key, value, lht);
    }
    return status;
}


