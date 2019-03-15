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

#if !defined(LINEAR_HASH_COMMON_TYPES_H)
#define LINEAR_HASH_COMMON_TYPES_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define LINEAR_HASH_BLOCK_SIZE 512
#define LINEAR_HASH_MAX_BLOCKS INT16_MAX
#define LINEAR_HASH_MAX_BUCKETS INT16_MAX
#define LINEAR_HASH_NO_OVERFLOW INT16_MAX

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
typedef uint16_t ion_linear_hash_block_index_t;

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

#if defined(__cplusplus)
}
#endif

#endif // End LINEAR_HASH_COMMON_TYPES_H
