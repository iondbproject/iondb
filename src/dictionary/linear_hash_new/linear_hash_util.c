//
// Created by itsal on 2019-02-13.
//

#include "linear_hash_util.h"

ion_err_t ion_linear_hash_insert_preconditions(ion_linear_hash_table_t *lht) {
    /* Verify we aren't overflowing the maximum size */
    if (UINT32_MAX == lht->num_records) {
        return err_out_of_bounds;
    }
    return err_ok;
}

ion_boolean_t
ion_linear_hash_check_above_threshold(
        ion_linear_hash_table_t *lht
) {
    float numerator = lht->num_records * 1.0;
    float load = (numerator / lht->current_size / lht->records_per_bucket) * 100;
    int load_int = (int) load;

#if LINEAR_HASH_DEBUG_INCREMENT
    printf("\tIncremented number of records to %lu, current size: %d, records per bucket: %d\n", (unsigned long) lht->num_records, lht->current_size, lht->records_per_bucket);
#ifdef ARDUINO

    char load_str[7];

    dtostrf(load, 5, 2, load_str);
    printf("\tLoad is now %s\n", load_str);
    printf("\tLoad is now (int) %d\n", load_int);
#else
    printf("\tLoad is now %.2f\n", load);
    printf("\tLoad is now (int) %d\n", load_int);
#endif
    printf("\tSplit threshold: %d\n", lht->split_threshold);
#endif

    if (load_int >= lht->split_threshold) {
        return boolean_true;
    }

    return boolean_false;
}

ion_linear_hash_block_index_t
ion_linear_hash_block_index_for_bucket(ion_linear_hash_bucket_index bucket, ion_linear_hash_table_t *linear_hash) {
    return ion_array_list_get((int) bucket, linear_hash->bucket_map);
}

ion_err_t
ion_linear_hash_save_block_index_for_bucket(ion_linear_hash_bucket_index bucket, ion_linear_hash_block_index_t block,
                                            ion_linear_hash_table_t *linear_hash) {
    return ion_array_list_insert((int) bucket, block, linear_hash->bucket_map);
}

ion_err_t ion_linear_hash_initialize_new_bucket_for_idx(ion_byte_t *block, ion_linear_hash_bucket_index idx,
                                                        ion_linear_hash_table_t *lht) {
    /* Preconditions */
    if (INT_MAX == lht->total_buckets) {
        return err_out_of_bounds;
    }

    memset(block, 0, LINEAR_HASH_BLOCK_SIZE);

    ion_linear_hash_bucket_t *bucket;

    bucket = (ion_linear_hash_bucket_t *) block;
    bucket->block = lht->next_block;
    lht->next_block++;
    bucket->records = 0;

    if (idx < lht->current_size) {
        int overflow = ion_linear_hash_block_index_for_bucket(idx, lht);

        bucket->overflow_block = overflow;
    } else {
        bucket->overflow_block = LINEAR_HASH_NO_OVERFLOW;
    }

    lht->total_buckets++;
    return ion_linear_hash_save_block_index_for_bucket(idx, bucket->block, lht);
}
