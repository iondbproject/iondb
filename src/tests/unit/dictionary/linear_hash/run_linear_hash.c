#include "test_linear_hash.h"

int
main(
) {
/*	runalltests_linear_hash(); */
/*	runalltests_flat_file_handler(); */
	ion_key_type_t		key_type			= key_type_numeric_signed;
	ion_key_size_t		key_size			= sizeof(int);
	ion_value_size_t	value_size			= sizeof(int);

	int					initial_size		= 5;
	int					split_threshold		= 85;
	int					records_per_bucket	= 4;
	linear_hash_table_t *linear_hash		= alloca(sizeof(linear_hash_table_t));
	array_list_t		*bucket_map			= alloca(sizeof(array_list_t));

	array_list_init(initial_size, bucket_map);

	ion_err_t err = linear_hash_init(1, key_type, key_size, value_size, initial_size, split_threshold, records_per_bucket, bucket_map, linear_hash);

	linear_hash->super.compare = dictionary_compare_signed_value;

	ion_byte_t	*key	= alloca(linear_hash->super.record.key_size);
	ion_byte_t	*value	= alloca(linear_hash->super.record.value_size);

	int *key_data		= alloca(linear_hash->super.record.key_size);
	int *value_data		= alloca(linear_hash->super.record.value_size);

	*key_data	= 10;
	*value_data = 10;
	memcpy(key, key_data, sizeof(linear_hash->super.record.key_size));
	memcpy(value, value_data, sizeof(linear_hash->super.record.value_size));

	linear_hash_insert(key, value, insert_hash_to_bucket(key, linear_hash), linear_hash);
	linear_hash_insert(key, value, insert_hash_to_bucket(key, linear_hash), linear_hash);
	linear_hash_insert(key, value, insert_hash_to_bucket(key, linear_hash), linear_hash);
	linear_hash_insert(key, value, insert_hash_to_bucket(key, linear_hash), linear_hash);
	linear_hash_insert(key, value, insert_hash_to_bucket(key, linear_hash), linear_hash);
	linear_hash_insert(key, value, insert_hash_to_bucket(key, linear_hash), linear_hash);

	for (int i = 0; i < linear_hash->num_buckets; i++) {
		print_linear_hash_bucket_from_idx(i, linear_hash);
	}

	print_linear_hash_state(linear_hash);
	return 0;
}
