/******************************************************************************/
/**
@file		linear_hash.h
@author		Andrew Feltham.
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

#if !defined(LINEAR_HASH_H)
#define LINEAR_HASH_H
#define LINEAR_HASH_DEBUG 0

#include <stdio.h>
#include "../dictionary_types.h"
#include "linear_hash_types.h"
#include "../../file/kv_stdio_intercept.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
@brief	Calculates the bucket index using h0 for a given key
@param[in]	key
            Pointer to the key to hash
@param[in]	linear_hash
            Pointer to the linear hash instance (required for knowledge of the key-size)
@return		An integer in the address space of the linear hash.
*/
int
ion_linear_hash_h0(
        int hash,
        ion_linear_hash_table_t *linear_hash
);

/**
@brief	Calculates the bucket index using h1 for a given key
@param[in]	key
				Pointer to the key to hash
@param[in]	linear_hash
				Pointer to the linear hash instance (required for knowledge of the key-size)
@return		An integer in the address space of the linear hash.
*/
int
ion_linear_hash_h1(
        int hash,
        ion_linear_hash_table_t *linear_hash
);

/**
 * @brief Reads a bucket block from the database file
 * @param [in] block [in] The integer block number to read
 * @param [in] linear_hash The linear hash instance containing the database to use
 * @param [out] buffer A buffer of size for a block that the bucket block will be read into
 * @return The error (if any)
 */
ion_err_t
ion_linear_hash_read_block(int block, ion_linear_hash_table_t *linear_hash, ion_byte_t *buffer);

ion_err_t ion_linear_hash_init(ion_dictionary_id_t id, ion_key_type_t key_type, ion_key_size_t key_size,
                               ion_value_size_t value_size, int initial_size, int split_threshold,
                               ion_linear_hash_table_t *linear_hash);

ion_status_t ion_linear_hash_insert(
        ion_key_t key,
        ion_value_t value,
        ion_linear_hash_table_t *linear_hash
);

ion_status_t ion_linear_hash_delete(
        ion_key_t key,
        ion_linear_hash_table_t *lht
);

ion_status_t ion_linear_hash_get(ion_key_t key, ion_value_t value, ion_linear_hash_table_t *linear_hash);

ion_status_t ion_linear_hash_update(ion_key_t key, ion_value_t value, ion_linear_hash_table_t *lht);

ion_err_t ion_linear_hash_split(
        ion_linear_hash_table_t *lht
);

/**
 * @brief Closes the linear hash table by saving the state and database, closing open files, and freeing buffer memory.
 * @param lht The linear hash table to close
 * @return The error if any
 */
ion_err_t ion_linear_hash_close(ion_linear_hash_table_t *lht);

/**
 * Simple hash function for integer values that casts the key to an int.
 * @param key The key to use
 * @param key_size The size of the key (not used)
 * @return the integer value of this key.
 */
int
ion_linear_hash_int_key_hash(ion_key_t key, ion_key_size_t key_size);

int
ion_linear_hash_generic_key_hash(ion_key_t key, ion_key_size_t key_size);

#if defined(__cplusplus)
}
#endif

#endif
