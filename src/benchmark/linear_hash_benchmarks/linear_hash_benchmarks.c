/*  */
/* Created by Spencer MacBeth on 2017-07-22. */
/*  */

#include "linear_hash_benchmarks.h"

void
run_benchmarks(
) {
	int					initial_size, split_threshold, records_per_bucket, bucket_idx;
	linear_hash_table_t *linear_hash = malloc(sizeof(linear_hash_table_t));

	initial_size		= 8;
	split_threshold		= 85;
	records_per_bucket	= 20;

	ion_err_t result = linear_hash_init(1, 8, key_type_numeric_signed, sizeof(int), sizeof(int), initial_size, split_threshold, records_per_bucket, linear_hash);

	if (result != err_ok) {
		printf("An error occured when creating the dictionary, error_code = %d", result);
	}

	ion_key_t	key		= IONIZE(17, int);
	ion_value_t value	= IONIZE(17, int);

	bucket_idx = insert_hash_to_bucket(key, linear_hash);

	ion_status_t status = linear_hash_insert(key, value, bucket_idx, linear_hash);

	printf("Finished insert, status = %d", status);
	linear_hash_destroy(linear_hash);
}
