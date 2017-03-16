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
	ion_key_t			key,
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
int
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
	linear_hash_bucket_t	bucket,
	linear_hash_table_t		*linear_hash
);

ion_fpos_t
create_overflow_bucket(
	int					bucket_idx,
	ion_fpos_t			*overflow_location,
	linear_hash_table_t *linear_hash
);

ion_fpos_t
get_bucket_records_location(
	ion_fpos_t bucket_loc
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

/* DEBUG METHODS */
void
print_array_list_data(
	array_list_t *array_list
);

void
print_all_linear_hash_index_buckets(
	int					idx,
	linear_hash_table_t *linear_hash
);

void
print_linear_hash_record(
	linear_hash_record_t record
);

void
print_linear_hash_bucket_from_idx(
	int					idx,
	linear_hash_table_t *linear_hash
);

void
print_linear_hash_state(
	linear_hash_table_t *linear_hash
);

/* linear_hash_record_t */
/* record_iterator_next( */
/*	linear_hash_record_iterator_t	*itr, */
/*	linear_hash_table_t				*linear_hash */
/* ); */

ion_err_t
linear_hash_destroy(
	linear_hash_table_t *linear_hash
);

ion_err_t
linear_hash_close(
	linear_hash_table_t *linear_hash
);
