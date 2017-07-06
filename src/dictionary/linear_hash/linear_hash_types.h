/******************************************************************************/
/**
@file		linear_hash_types.h
@author		Spencer MacBeth
@brief		Header for linear hash types.
@copyright	Copyright 2017
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

#include <stdio.h>
#include "../../key_value/kv_system.h"
#include "../dictionary.h"
#include "../../file/sd_stdio_c_iface.h"

typedef ion_byte_t *linear_hash_record_status_t;

#define linear_hash_end_of_list			-1
#define linear_hash_record_status_empty 0
#define linear_hash_record_status_full	1

/* SIMPLE ARRAY_LIST FOR BUCKET MAP */
typedef struct {
	int			current_size;
	ion_fpos_t	*data;
} array_list_t;

/* definition of linear hash record, with a type and pointer instance declared for later use */
typedef struct {
	ion_key_t	key;
	ion_value_t value;
} linear_hash_record_t;

/* buckets */
typedef struct {
	int			idx;
	int			record_count;
	ion_fpos_t	overflow_location;
} linear_hash_bucket_t;

/* function pointer syntax: return_type (*function_name) (arg_type) */
/* linear hash structure definition, with a type and pointer instance declared for later use */
typedef struct {
	/**> Parent structure that holds dictionary level information. */
	ion_dictionary_parent_t super;
	ion_dictionary_size_t	dictionary_size;
	int						initial_size;
	int						next_split;
	int						split_threshold;
	int						num_buckets;
	int						num_records;
	int						records_per_bucket;
	ion_fpos_t				record_total_size;
	FILE					*database;
	FILE					*state;

	/* maps the location of the head of the linked list of buckets corresponding to its index */
	array_list_t			*bucket_map;

	/* generic cache */
	ion_byte_t				*cache;
	int						last_cache_idx;

	/* pointer location of the next record to swap-on-delete*/
	ion_fpos_t				swap_bucket_loc;
} linear_hash_table_t;

/* typedef struct { */
/*	ion_fpos_t	next; */
/*	ion_fpos_t	current_bucket_loc; */
/* } linear_hash_record_iterator_t; */
