/******************************************************************************/
/**
@file		linear_hash_file.h
@author		Feltham
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

#ifndef IONDB_LINEAR_HASH_FILE_H
#define IONDB_LINEAR_HASH_FILE_H

#include "linear_hash_types.h"
#include "linear_hash_macros.h"
#include "linear_hash_util.h"
#include "array_list.h"
#include <stdio.h>
#include "../../file/kv_stdio_intercept.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Reads a bucket block from the database file
 * @param [in] block [in] The integer block number to read
 * @param [in] linear_hash The linear hash instance containing the database to use
 * @param [out] buffer A buffer of size for a block that the bucket block will be read into
 * @return The error (if any)
 */
ion_err_t
ion_linear_hash_read_block(int block, ion_linear_hash_table_t *linear_hash, ion_byte_t *buffer);

/**
 * @brief Reads a bucket block from the database file
 * @param [in] block [in] The integer block number to read
 * @param [in] linear_hash The linear hash instance containing the database to use
 * @param [out] buffer A buffer of size for a block that the bucket block will be read into
 * @return The error (if any)
 */
ion_err_t
ion_linear_hash_read_block_file(ion_linear_hash_block_index_t block, FILE *file, ion_byte_t *buffer);

/**
 * @brief Writes a bucket block to the database file, overwriting the current block
 * @param [in] bucket The bucket to write
 * @param [in] block The block number to write
 * @param [in] linear_hash The linear hash instance containing the database to use
 * @return The error (if any)
 */
ion_err_t
ion_linear_hash_write_block(ion_byte_t *bucket, int block, ion_linear_hash_table_t *linear_hash);

ion_err_t
ion_linear_hash_write_block_file(ion_byte_t *data, ion_linear_hash_block_index_t block, FILE *file);

/**
 * @brief Reads a top level bucket block to a buffer using a given top level bucket index.
 * @param linear_hash The linear hash table in use.
 * @param bucket_index The bucket index
 * @return The buffer containing the block and error state.
 */
ion_linear_hash_buffer_t *
ion_linear_hash_read_data_block(ion_linear_hash_table_t *linear_hash, ion_linear_hash_buffer_t *buffer,
                                ion_linear_hash_bucket_index bucket_index);

/**
 * @brief Reads an overflow block using the specified block index.
 * @param linear_hash The linear hash table in use.
 * @param block_index The index of the overflow block.
 * @return The buffer containing the block or error state.
 */
ion_linear_hash_buffer_t *
ion_linear_hash_read_overflow_block(ion_linear_hash_table_t *linear_hash, ion_linear_hash_buffer_t *buffer,
                                    ion_linear_hash_block_index_t block_index);

ion_err_t
ion_linear_hash_save_state(
        ion_linear_hash_table_t *table
);

ion_err_t
ion_linear_hash_read_state(
        ion_linear_hash_table_t *table
);

ion_err_t
ion_linear_hash_write_buffer(ion_linear_hash_table_t *linear_hash_table, ion_linear_hash_buffer_t *buffer);

#if defined(__cplusplus)
}
#endif

#endif //IONDB_LINEAR_HASH_FILE_H
