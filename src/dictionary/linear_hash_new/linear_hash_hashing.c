#include "../../../../../../cygwin64/usr/include/endian.h"//
// Created by itsal on 2019-02-13.
//

#include "linear_hash_hashing.h"

ion_linear_hash_key_hash
ion_linear_hash_sdbm_hash(
        ion_key_t key,
        ion_key_size_t key_size
) {
    ion_byte_t *key_bytes = (ion_byte_t *) key;
    uint32_t hash = 0;
    for (int i = 0; i < key_size; i++) {
        hash = key_bytes[i] + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

ion_linear_hash_bucket_index
ion_linear_hash_int_key_hash(
        ion_key_t key,
        ion_key_size_t __unused key_size
) {
    return *(ion_linear_hash_bucket_index *) key;
}

ion_linear_hash_bucket_index
ion_linear_hash_key_to_bucket_idx(
        ion_key_t key,
        ion_linear_hash_table_t *linear_hash
) {
    ion_linear_hash_key_hash hash = linear_hash->hash_key_function(key, linear_hash->super.record.key_size);
    ion_linear_hash_bucket_index idx = ion_linear_hash_h0(hash, linear_hash);
    if (idx < linear_hash->next_split) {
        idx = ion_linear_hash_h1(hash, linear_hash);
    }
    return idx;
}

ion_linear_hash_bucket_index
ion_linear_hash_h0(
        ion_linear_hash_key_hash hash,
        ion_linear_hash_table_t *linear_hash
) {
    return hash & (linear_hash->initial_size - 1);
}

ion_linear_hash_bucket_index
ion_linear_hash_h1(
        ion_linear_hash_key_hash hash,
        ion_linear_hash_table_t *linear_hash
) {
    return hash & ((2 * linear_hash->initial_size) - 1);
}