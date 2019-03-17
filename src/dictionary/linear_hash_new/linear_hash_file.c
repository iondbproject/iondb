#include "linear_hash_file.h"

ion_err_t
ion_linear_hash_read_block(
        ion_linear_hash_block_index_t block,
        ion_linear_hash_table_t *linear_hash,
        ion_byte_t *buffer
) {
    unsigned long offset = ((unsigned long) block) * (LINEAR_HASH_BLOCK_SIZE);

    LH_READ_BLOCK_DEBUG_PRINT("Reading block %d from offset %lu\n", block, offset);

    if (0 != fseek(linear_hash->database, offset, SEEK_SET)) {
        LH_READ_BLOCK_DEBUG_PRINT("Failed to seek %lu\n", offset);
        return err_file_bad_seek;
    }

    if (1 != fread(buffer, LINEAR_HASH_BLOCK_SIZE, 1, linear_hash->database)) {
        LH_READ_BLOCK_DEBUG_PRINT("Failed to read block %d\n", block);
        return err_file_read_error;
    }

    linear_hash->ion_linear_hash_block_reads++;
    return err_ok;
}

ion_err_t
ion_linear_hash_write_block(
        ion_byte_t *bucket,
        int block,
        ion_linear_hash_table_t *linear_hash
) {
    unsigned long offset = ((unsigned long) block) * LINEAR_HASH_BLOCK_SIZE;

    LH_WRITE_BLOCK_DEBUG_PRINT("Writing block %d to offset %lu\n", block, offset);

    if (0 != fseek(linear_hash->database, offset, SEEK_SET)) {
        LH_WRITE_BLOCK_DEBUG_PRINT("Failed to seek %lu\n", offset);
        return err_file_bad_seek;
    }

    if (1 != fwrite(bucket, LINEAR_HASH_BLOCK_SIZE, 1, linear_hash->database)) {
        LH_WRITE_BLOCK_DEBUG_PRINT("Failed to write block %d to offset %lu\n", block, offset);
        return err_file_write_error;
    }

    linear_hash->ion_linear_hash_block_writes++;
    return err_ok;
}

ion_err_t
ion_linear_hash_write_block_file(
        ion_byte_t *data,
        ion_linear_hash_block_index_t block,
        FILE *file
) {
    unsigned long offset = ((unsigned long) block) * LINEAR_HASH_BLOCK_SIZE;

    LH_WRITE_BLOCK_DEBUG_PRINT("Writing block %d to offset %lu\n", block, offset);

    if (0 != fseek(file, offset, SEEK_SET)) {
        LH_WRITE_BLOCK_DEBUG_PRINT("Failed to seek %lu\n", offset);
        return err_file_bad_seek;
    }

    if (1 != fwrite(data, LINEAR_HASH_BLOCK_SIZE, 1, file)) {
        LH_WRITE_BLOCK_DEBUG_PRINT("Failed to write block %d to offset %lu\n", block, offset);
        return err_file_write_error;
    }

    return err_ok;
}

ion_err_t
ion_linear_hash_save_state(
        ion_linear_hash_table_t *table
) {
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
ion_linear_hash_read_state(
        ion_linear_hash_table_t *table
) {
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

ion_err_t
ion_linear_hash_read_block_file(
        ion_linear_hash_block_index_t block,
        FILE *file,
        ion_byte_t *buffer
) {
    unsigned long offset = ((unsigned long) block) * (LINEAR_HASH_BLOCK_SIZE);

    LH_READ_BLOCK_DEBUG_PRINT("Reading block %d from offset %lu\n", block, offset);

    if (0 != fseek(file, offset, SEEK_SET)) {
        LH_READ_BLOCK_DEBUG_PRINT("\tFailed to seek %lu\n", offset);
        return err_file_bad_seek;
    }

    if (1 != fread(buffer, LINEAR_HASH_BLOCK_SIZE, 1, file)) {
        LH_READ_BLOCK_DEBUG_PRINT("\tFailed to read block %d\n", block);
        return err_file_read_error;
    }

    return err_ok;
}

ion_linear_hash_buffer_t *
ion_linear_hash_read_overflow_block(
        ion_linear_hash_table_t *linear_hash,
        ion_linear_hash_block_index_t block_index
) {
    ion_err_t err;
    ion_linear_hash_buffer_t *buffer = linear_hash->buffer1;

    err = ion_linear_hash_read_block_file(block_index, linear_hash->database, buffer->block.raw);
    buffer->err = err;
    buffer->type = OVERFLOW;
    buffer->dirty = boolean_false;
    buffer->block_index = block_index;

    if (err_ok == err) {
        linear_hash->ion_linear_hash_block_reads++;
    }

    return buffer;
}

ion_linear_hash_buffer_t *
ion_linear_hash_read_data_block(ion_linear_hash_table_t *linear_hash, ion_linear_hash_bucket_index bucket_index) {
    ion_linear_hash_buffer_t *buffer = linear_hash->buffer1;
    ion_linear_hash_block_index_t block = ion_linear_hash_block_index_for_bucket(bucket_index, linear_hash);
    ion_err_t err;

    if (ARRAY_LIST_END_OF_LIST == block) {
        err = err_out_of_bounds;
    } else {
        err = ion_linear_hash_read_block_file(block, linear_hash->database, buffer->block.raw);

        if (err_ok == err) {
            linear_hash->ion_linear_hash_block_reads++;
        }
    }

    buffer->err = err;
    buffer->block_index = block;
    buffer->type = DATA;
    buffer->dirty = boolean_false;
    return buffer;
}

ion_err_t
ion_linear_hash_write_buffer(
        ion_linear_hash_table_t *linear_hash_table,
        ion_linear_hash_buffer_t *buffer
) {
    ion_err_t err;
    if (boolean_true == buffer->dirty) {
        if ((DATA == buffer->type) || (OVERFLOW == buffer->type)) {
            err = ion_linear_hash_write_block_file(buffer->block.raw, buffer->block_index, linear_hash_table->database);
            if (err_ok == err) {
                buffer->dirty = boolean_false;
                linear_hash_table->ion_linear_hash_block_writes++;
            }
        } else {
            err = err_file_write_error;
        }
    } else {
        err = err_ok;
    }
    return err;
}