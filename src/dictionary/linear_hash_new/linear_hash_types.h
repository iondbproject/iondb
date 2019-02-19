/******************************************************************************/
/**
@file		linear_hash_types.h
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

#if !defined(LINEAR_HASH_TYPES_H)
#define LINEAR_HASH_TYPES_H

#include <stdio.h>
#include "../../file/kv_stdio_intercept.h"
#include "../../key_value/kv_system.h"
#include "../../dictionary/dictionary_types.h"
#include "../../file/sd_stdio_c_iface.h"
#include "array_list.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define LINEAR_HASH_BLOCK_SIZE 512
#define LINEAR_HASH_NO_OVERFLOW -1

/**
 * Defines the type for a hashed key
 */
typedef uint32_t ion_linear_hash_key_hash;

/**
 * Defines the type for the linear hash table bucket index for retrieving a top level bucket.
 */
typedef uint32_t ion_linear_hash_bucket_index;

/**
 * Defines the type for the block index
 */
typedef int ion_linear_hash_block_index_t;

enum buffer_type {
    /**
       * The buffer doesn't contain any reasonable data
       */
            EMPTY,
    /**
     * The buffer is a top level linear hash block.
     */
            DATA,

    /**
     * The buffer is an overflow block
     */
            OVERFLOW,

    /**
     * The block is for state or bucket maps
     */
            STATE,
};

/**
 * A buffer to keep track of the type
 */
typedef struct {
    /**
     * Stores the 512 buffer that was read from the file.
     */
    ion_byte_t data[LINEAR_HASH_BLOCK_SIZE];

    /**
     * The type of buffer that was loaded.
     */
    enum buffer_type type;

    /**
     * Indicates if the block needs to be written to the file.
     */
    ion_boolean_t dirty;

    /**
     * The block index in the file where this block belongs.
     */
    ion_linear_hash_block_index_t block_index;

    /**
     * The error state of the last read
     */
    ion_err_t err;

} ion_linear_hash_buffer_t;

/**
 * Represents a linear hash table data
 */
typedef struct {
    /**
     * The parent directly handler
     */
    ion_dictionary_parent_t super;

    /**
     * The initial size of the dictionary and the size of the direction when it has doubled. This is used to know when
     * to reset the split bucket.
     */
    ion_linear_hash_bucket_index initial_size;

    /**
     * The next bucket that will be split
     */
    ion_linear_hash_bucket_index next_split;

    /**
     * The integer percent threshold that determines when a split is made.
     */
    int split_threshold;

    /**
     * The current number of top level buckets in the linear hash table.
     */
    ion_linear_hash_block_index_t current_size;

    /**
     * The total number of buckets in the linear hash table including overflow buckets.
     */
    ion_linear_hash_block_index_t total_buckets;

    /**
     * The total number of records in the hash table in all top level buckets and all overflow buckets.
     */
    uint32_t num_records;

    /**
     * The total number of records that can be stored in a bucket. Automatically calculated from the size of the bucket
     * header, the size of each record and the total size of the block.
     */
    int records_per_bucket;

    /**
     * The total count of blocks in use. This is used to know the next block in the file to use when a new bucket is
     * created.
     */
    ion_linear_hash_block_index_t next_block;

    /**
     * Convience value for the total size (key + value) of a record in bytes.
     */
    int record_total_size;

    /**
     * The first block cache used to load and write bucket blocks to the file
     */
    ion_linear_hash_buffer_t *buffer1;

    /**
     * The second block cache used to load and write bucket blocks to the file.
     */
    ion_linear_hash_buffer_t *buffer2;

    /**
     * The array list containing the index to block mapping
     */
    ion_array_list_t *bucket_map;

    /**
     * The database file
     */
    FILE *database;

    /**
     * The state file.
     */
    FILE *state;

    /**
     * The hashing function for a key to an int
     *
     * Some pre-made hashing functions are provided and will automatically be used based on the type of key that is
     * provided to the dictionary.
     *
     * @param key The key to hash to an integer value
     */
    ion_linear_hash_key_hash (*hash_key_function)(ion_key_t key, ion_key_size_t key_size);

    unsigned long ion_linear_hash_block_reads;

    unsigned long ion_linear_hash_block_writes;
} ion_linear_hash_table_t;

/**
 * Bucket header that stores the block index, next block in the chain, and number of records
 */
typedef struct {
    ion_linear_hash_block_index_t block;
    int records;
    ion_linear_hash_block_index_t overflow_block;
} ion_linear_hash_bucket_t;

#if defined(__cplusplus)
}
#endif

#endif // End LINEAR_HASH_TYPES_H
