/**
@file		linear_hash_hashing.h
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
#ifndef IONDB_LINEAR_HASH_HASHING_H
#define IONDB_LINEAR_HASH_HASHING_H

#include "linear_hash_types.h"

/**
 * Simple hash function for integer values that casts the key to an int.
 * @param key The key to use
 * @param key_size The size of the key (not used)
 * @return the integer value of this key.
 */
ion_linear_hash_bucket_index
ion_linear_hash_int_key_hash(ion_key_t key, ion_key_size_t key_size);


/**
 * Implementation of sdbm hash for generic key hashing
 * @param key The key to hash
 * @param key_size The size of the key to hash
 * @return A hashed uint32 integer
 */
ion_linear_hash_key_hash
ion_linear_hash_sdbm_hash(ion_key_t key, ion_key_size_t key_size);

/**
@brief	Calculates the bucket index using h0 for a given key
@param[in]	key
            Pointer to the key to hash
@param[in]	linear_hash
            Pointer to the linear hash instance (required for knowledge of the key-size)
@return		An integer in the address space of the linear hash.
*/
ion_linear_hash_bucket_index
ion_linear_hash_h0(
        ion_linear_hash_key_hash hash,
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
ion_linear_hash_bucket_index
ion_linear_hash_h1(
        ion_linear_hash_key_hash hash,
        ion_linear_hash_table_t *linear_hash
);

/**
 * @brief Hashes a key to a bucket index value using the linear hash function set in the
 * linear hash table and the h0 and h1 functions
 * @param key The key hash
 * @param linear_hash The linear hash table
 * @return The bucket index for the given key.
 */
ion_linear_hash_bucket_index
ion_linear_hash_key_to_bucket_idx(ion_key_t key, ion_linear_hash_table_t *linear_hash);

#endif //IONDB_LINEAR_HASH_HASHING_H
