//
// Created by itsal on 2019-02-13.
//

#ifndef IONDB_LINEAR_HASH_UTIL_H
#define IONDB_LINEAR_HASH_UTIL_H

#include <limits.h>
#include "linear_hash_types.h"
#include "linear_hash_macros.h"

ion_err_t
ion_linear_hash_insert_preconditions(
        ion_linear_hash_table_t *lht
);

/**
 * @brief Returns boolean_true if the linear hash table load is above the threshold.
 */
ion_boolean_t
ion_linear_hash_check_above_threshold(ion_linear_hash_table_t *lht);

/**
 * @brief Gets and returns the file block index where the top level bucket for the given index resides.
 * @param bucket The index of the top level bucket to retrieve.
 * @return The location of the bucket to use
 */
ion_linear_hash_block_index_t
ion_linear_hash_block_index_for_bucket(ion_linear_hash_bucket_index bucket, ion_linear_hash_table_t *linear_hash);

/**
 * @brief Saves the block index for a top level bucket into a retrievable storage.
 * @param bucket
 * @param linear_hash
 */
ion_err_t
ion_linear_hash_save_block_index_for_bucket(ion_linear_hash_bucket_index bucket, ion_linear_hash_block_index_t block,
                                            ion_linear_hash_table_t *linear_hash);

/**
 * Initializes the memory block to a new, empty block with no records.
 * @param block_num
 * @param buffer
 * @param block_size
 * @param overflow_block
 */
ion_err_t
ion_linear_hash_initialize_new_bucket_for_idx(ion_linear_hash_buffer_t *buffer, ion_linear_hash_bucket_index idx,
                                              ion_linear_hash_table_t *lht, enum buffer_type buffer_type);

/**
 * @brief Sets the buffer block to the next available block and sets it to be dirty
 * @param buffer
 * @param hash_table
 * @return err_max_capacity when the table has exceeded the limit of addressable blocks
 */
ion_err_t
ion_linear_hash_increment_buffer_block(ion_linear_hash_buffer_t *buffer, ion_linear_hash_table_t *hash_table);

#endif //IONDB_LINEAR_HASH_UTIL_H
