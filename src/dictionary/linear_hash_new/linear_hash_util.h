//
// Created by itsal on 2019-02-13.
//

#ifndef IONDB_LINEAR_HASH_UTIL_H
#define IONDB_LINEAR_HASH_UTIL_H

#include <limits.h>
#include "linear_hash_types.h"
#include "linear_hash_macros.h"
#include "array_list.h"

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
 * Initializes the memory block to a new, empty block with no records.
 * @param block_num
 * @param buffer
 * @param block_size
 * @param overflow_block
 */
ion_err_t
ion_linear_hash_initialize_new_bucket_for_idx(ion_linear_hash_buffer_t *buffer, ion_linear_hash_bucket_index idx,
                                              ion_linear_hash_table_t *lht, enum buffer_type buffer_type);

#endif //IONDB_LINEAR_HASH_UTIL_H
