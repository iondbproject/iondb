/******************************************************************************/
/**
@file		linear_hash.h
@author		Spencer MacBeth
@brief		Header for a linear hash .
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
#include "linear_hash_types.h"

#if defined(ARDUINO)
#include "../../serial/serial_c_iface.h"
#endif

ion_err_t
linear_hash_init(
	ion_dictionary_id_t		id,
	ion_dictionary_size_t	dictionary_size,
	ion_key_type_t			key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	int						initial_size,
	int						split_threshold,
	int						records_per_bucket,
	linear_hash_table_t		*linear_hash
);

ion_err_t
linear_hash_read_state(
	linear_hash_table_t *linear_hash
);

ion_err_t
linear_hash_write_state(
	linear_hash_table_t *linear_hash
);

ion_err_t
linear_hash_increment_num_records(
	linear_hash_table_t *linear_hash
);

/* decrement the count of the records stored in the linear hash */
ion_err_t
linear_hash_decrement_num_records(
	linear_hash_table_t *linear_hash
);

ion_err_t
linear_hash_increment_num_buckets(
	linear_hash_table_t *linear_hash
);

ion_err_t
linear_hash_update_state(
	linear_hash_table_t *linear_hash
);

int
linear_hash_bucket_is_full(
	linear_hash_bucket_t	bucket,
	linear_hash_table_t		*linear_hash
);

/* split function */
ion_err_t
split(
	linear_hash_table_t *linear_hash
);

ion_status_t
linear_hash_insert(
	ion_key_t			key,
	ion_value_t			value,
	int					hash_bucket_idx,
	linear_hash_table_t *linear_hash
);

/* linear hash operations */
ion_status_t
linear_hash_get(
	ion_byte_t			*key,
	ion_byte_t			*value,
	linear_hash_table_t *linear_hash
);

ion_status_t
linear_hash_update(
	ion_key_t			key,
	ion_value_t			value,
	linear_hash_table_t *linear_hash
);

ion_status_t
linear_hash_delete(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
);

ion_err_t
linear_hash_get_record(
	ion_fpos_t			loc,
	ion_byte_t			*key,
	ion_byte_t			*value,
	ion_byte_t			*status,
	linear_hash_table_t *linear_hash
);

ion_err_t
linear_hash_write_record(
	ion_fpos_t			record_loc,
	ion_byte_t			*key,
	ion_byte_t			*value,
	ion_byte_t			*status,
	linear_hash_table_t *linear_hash
);

/* check if linear hash is above its split threshold */
ion_boolean_t
linear_hash_above_threshold(
	linear_hash_table_t *linear_hash
);

/* BUCKET OPERATIONS */
ion_err_t
write_new_bucket(
	int					idx,
	linear_hash_table_t *linear_hash
);

/* returns the struct representing the bucket at the specified index */
ion_err_t
linear_hash_get_bucket(
	ion_fpos_t				bucket_loc,
	linear_hash_bucket_t	*bucket,
	linear_hash_table_t		*linear_hash
);

ion_err_t
linear_hash_update_bucket(
	ion_fpos_t				bucket_loc,
	linear_hash_bucket_t	*bucket,
	linear_hash_table_t		*linear_hash
);

ion_err_t
create_overflow_bucket(
	int					bucket_idx,
	ion_fpos_t			*overflow_location,
	linear_hash_table_t *linear_hash
);

ion_fpos_t
get_bucket_records_location(
	ion_fpos_t bucket_loc
);

ion_err_t
invalidate_buffer_records(
	ion_byte_t			*key,
	int					record_count,
	ion_byte_t			*records,
	linear_hash_table_t *linear_hash
);

/* hash methods */
int
hash_to_bucket(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
);

int
insert_hash_to_bucket(
	ion_byte_t			*key,
	linear_hash_table_t *linear_hash
);

/* returns the struct representing the bucket at the specified index */
linear_hash_bucket_t
linear_hash_get_overflow_bucket(
	ion_fpos_t loc
);

/* Returns the file offset where bucket with index idx begins */
ion_fpos_t
bucket_idx_to_ion_fpos_t(
	int					idx,
	linear_hash_table_t *linear_hash
);

ion_err_t
linear_hash_increment_next_split(
	linear_hash_table_t *linear_hash
);

void
print_linear_hash_bucket(
	linear_hash_bucket_t bucket
);

void
print_linear_hash_bucket_map(
	linear_hash_table_t *linear_hash
);

/* Write the offset of bucket idx to the map in linear hash state */
ion_err_t
store_bucket_loc_in_map(
	int					idx,
	ion_fpos_t			bucket_loc,
	linear_hash_table_t *linear_hash
);

/* ARRAY_LIST METHODS */
ion_err_t
array_list_init(
	int				init_size,
	array_list_t	*array_list
);

ion_err_t
array_list_insert(
	int				bucket_idx,
	ion_fpos_t		bucket_loc,
	array_list_t	*array_list
);

ion_fpos_t
array_list_get(
	int				bucket_idx,
	array_list_t	*array_list
);

ion_err_t
linear_hash_destroy(
	linear_hash_table_t *linear_hash
);

ion_err_t
linear_hash_close(
	linear_hash_table_t *linear_hash
);

void
print_linear_hash_distribution(
	linear_hash_table_t *linear_hash
);
