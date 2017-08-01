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
benchmark_insert_step(
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
	unsigned long			total_start = ion_time();
	unsigned long			start;

	printf("\nBeggining trial with %d upper bound\n\n", record_count);

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

	unsigned long elapsed = ion_time() - total_start;

	printf("\nTook %ld to insert all %d records \n", elapsed, record_count);
	linear_hash_destroy(linear_hash);
}

void
benchmark_insert(
        int record_count
) {
    int					initial_size, split_threshold, records_per_bucket, bucket_idx;
    linear_hash_table_t *linear_hash = malloc(sizeof(linear_hash_table_t));

    initial_size		= 8;
    split_threshold		= 85;
    records_per_bucket	= 20;
    int key_size = sizeof(int);
    int value_size = 20;
    ion_err_t result = linear_hash_init(1, 8, key_type_numeric_signed, key_size, value_size, initial_size, split_threshold, records_per_bucket, linear_hash);

    linear_hash->super.compare = dictionary_compare_signed_value;

    if (result != err_ok) {
        printf("An error occured when creating the dictionary, error_code = %d\n", result);
    }

    int				i;
    ion_status_t	status;
    ion_key_t		key;
    ion_value_t		value = alloca(value_size);
    memset(value, 0, 20);
    unsigned long			start;
    unsigned long tracker_start;   // used to track the max time for operation of interest
    unsigned long tracker_elapsed;
    unsigned long tracker_max = 0;
    printf("\nBeggining INSERT trial with %d records\n\n", record_count);

    start = ion_time();
    for (i = 0; i < record_count; i++) {
        key			= IONIZE(rand(), int);
        bucket_idx	= insert_hash_to_bucket(key, linear_hash);
        tracker_start = ion_time();
        status		= linear_hash_insert(key, value, bucket_idx, linear_hash);
        tracker_elapsed = ion_time() - tracker_start;
        if (tracker_elapsed > tracker_max) {
            tracker_max = tracker_elapsed;
        }
        if (status.error != err_ok) {
            printf("An error occurred, error code = %d\n", status.error);
        }
    }
    unsigned long elapsed = ion_time() - start;
    printf("\nTook %ld to insert all %d records, max time = %ld\n", elapsed, record_count, tracker_max);
    linear_hash_destroy(linear_hash);
}

void
benchmark_get(
        int record_count,
        int get_count
) {
    int					initial_size, split_threshold, records_per_bucket, bucket_idx;
    linear_hash_table_t *linear_hash = malloc(sizeof(linear_hash_table_t));

    initial_size		= 8;
    split_threshold		= 85;
    records_per_bucket	= 20;
    int key_size = sizeof(int);
    int value_size = 20;
    ion_err_t result = linear_hash_init(1, 8, key_type_numeric_signed, key_size, value_size, initial_size, split_threshold, records_per_bucket, linear_hash);

    linear_hash->super.compare = dictionary_compare_signed_value;

    if (result != err_ok) {
        printf("An error occured when creating the dictionary, error_code = %d\n", result);
    }

    int				i;
    ion_status_t	status;
    ion_key_t		key;
    ion_value_t		value = alloca(value_size);
    memset(value, 0, 20);
    unsigned long			start;
    unsigned long tracker_start;   // used to track the max time for operation of interest
    unsigned long tracker_elapsed;
    unsigned long tracker_max = 0;

    printf("\nFilling linear hash with %d records for GET benchmark\n", record_count);
    for (i = 0; i < record_count; i++) {
        key			= IONIZE(rand(), int);
        bucket_idx	= insert_hash_to_bucket(key, linear_hash);
        status		= linear_hash_insert(key, value, bucket_idx, linear_hash);
        if (status.error != err_ok) {
            printf("An error occurred, error code = %d\n", status.error);
        }
    }

    printf("\nBeggining GET trial for %d records...\n\n", get_count);
    srand((unsigned) ion_time()); // seed rng with different seed than inserts
    start = ion_time();
    for (i = 0; i < get_count; i++) {
        key			= IONIZE(rand(), int);
        tracker_start = ion_time();
        status		= linear_hash_get(key, value, linear_hash);
        tracker_elapsed = ion_time() - tracker_start;
        if (tracker_elapsed > tracker_max) {
            tracker_max = tracker_elapsed;
        }
    }
    unsigned long elapsed = ion_time() - start;
    printf("\nTook %ld to get all %d records, max time = %ld\n", elapsed, get_count, tracker_max);
    linear_hash_destroy(linear_hash);
}

void
run_benchmarks(
) {
	benchmark_insert(1000);
	benchmark_insert(10000);
	benchmark_insert(100000);
    benchmark_get(1000, 100);
    benchmark_get(10000, 100);
    benchmark_get(100000, 100);
}
