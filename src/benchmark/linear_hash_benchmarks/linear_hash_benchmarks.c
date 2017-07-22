/*  */
/* Created by Spencer MacBeth on 2017-07-22. */
/*  */

#include "linear_hash_benchmarks.h"
#include "./../../tests/planck-unit/src/ion_time/ion_time.h"

void
print_header(
	char *headerType
) {
	if (strcmp(headerType, "insert")) {
		printf("time_elapsed,record_count\n");
	}
}

void
benchmark_insert(
	int record_count,
	int step_size
) {
	int					initial_size, split_threshold, records_per_bucket, bucket_idx;
	linear_hash_table_t *linear_hash = malloc(sizeof(linear_hash_table_t));

	initial_size		= 8;
	split_threshold		= 85;
	records_per_bucket	= 20;

	ion_err_t result = linear_hash_init(1, 8, key_type_numeric_signed, sizeof(int), sizeof(int), initial_size, split_threshold, records_per_bucket, linear_hash);

	linear_hash->super.compare = dictionary_compare_signed_value;

	if (result != err_ok) {
		printf("An error occured when creating the dictionary, error_code = %d\n", result);
	}

	int				i, n;
	int				outer_iterations = ((int) ((double) record_count) / step_size);
	ion_status_t	status;
	ion_key_t		key;
	ion_value_t		value;
	time_t			total_start = ion_time();
	time_t			start;

	printf("\nBeggining trail with %d upper bound\n\n", record_count);

	for (i = 0; i < outer_iterations; i++) {
		start = ion_time();

		for (n = 0; n < step_size; n++) {
			key			= IONIZE(rand(), int);
			value		= IONIZE(rand(), int);
			bucket_idx	= insert_hash_to_bucket(key, linear_hash);
			status		= linear_hash_insert(key, value, bucket_idx, linear_hash);
		}

		if (status.error != err_ok) {
			printf("An error occurred, error code = %d\n", status.error);
		}

		printf("%ld,%d\n", ion_time() - start, linear_hash->num_records);
	}

	time_t elapsed = ion_time() - total_start;

	printf("\nTook %ld to insert all %d records \n", elapsed, record_count);
	linear_hash_destroy(linear_hash);
}

void
run_benchmarks(
) {
	benchmark_insert(100000, 100);
}
