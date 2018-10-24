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

typedef struct {
    ion_dictionary_parent_t super;
    ion_dictionary_size_t dictionary_size;
    int initial_size;
    int next_split;
    int split_threshold;
    int num_buckets;
    int num_records;
    int records_per_bucket;
    int total_blocks;
    ion_fpos_t record_total_size;
    ion_byte_t *block1;
    ion_byte_t *block2;
    ion_array_list_t *bucket_map;
    FILE *database;
    FILE *state;
} ion_linear_hash_table_t;

typedef struct {
    int block;
    int records;
    int overflow_block;
} ion_linear_hash_bucket_t;

#if defined(__cplusplus)
}
#endif

#endif // End LINEAR_HASH_TYPES_H
