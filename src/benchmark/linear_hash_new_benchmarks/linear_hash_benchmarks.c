#include <stdio.h>
#include "../../file/kv_stdio_intercept.h"
#include "linear_hash_benchmarks.h"
#include "./../../tests/planck-unit/src/ion_time/ion_time.h"

long max_insert = 100000;

ion_dictionary_id_t dictionary_id = 0;

char str[200];

lfsr_t lfsr;

struct bucket_stats {
    int idx;
    int num_buckets;
    int num_records;
};

struct bucket_stats
bench_index_chain_stats(int idx, ion_linear_hash_table_t *table) {
    int block = ion_array_list_get(idx, table->bucket_map);
    int num_blocks = 0;
    int num_records = 0;
    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) table->block1;
    ion_linear_hash_read_block(block, table, table->block1);
    ion_boolean_t terminal = boolean_true;
    while (terminal) {
        num_blocks++;
        num_records += bucket->records;

        if (bucket->overflow_block == LINEAR_HASH_NO_OVERFLOW) {
            terminal = boolean_false;
        } else {
            ion_linear_hash_read_block(bucket->overflow_block, table, table->block1);
        }
    }
    struct bucket_stats stats;
    stats.idx = idx;
    stats.num_records = num_records;
    stats.num_buckets = num_blocks;
    return stats;
}

ion_dictionary_t *
bench_dictionary_create(ion_dictionary_handler_t *handler) {
    ion_dictionary_t *dict = malloc(sizeof(ion_dictionary_t));
    char name[ION_MAX_FILENAME_LENGTH];
    dictionary_get_filename(dictionary_id, ".lhs", name);
    fremove(name);
    dictionary_get_filename(dictionary_id, ".lhd", name);
    fremove(name);
    ion_err_t err = dictionary_create(handler, dict, dictionary_id++, key_type_numeric_signed, sizeof(int), sizeof(int),
                                      4);
    if (err_ok != err) {
        free(dict);
        printf("Unable to create dictionary. Err %d\n", err);
        return NULL;
    }
    return dict;
}

/**
 * Inserts a random key and value count times into the dict and returns the time taken.
 *
 * @param dict The dict to use
 * @param count The number of records to insert.
 */
unsigned long
bench_insert_records_random(ion_dictionary_t *dict, long count) {
    int key;
    int value;
    long start = ion_time();
    for (long i = 0; i < count; ++i) {
        key = lfsr_get_next(&lfsr);
        value = lfsr_get_next(&lfsr);
        dictionary_insert(dict, IONIZE(key, int), IONIZE(value, int));
    }
    return ion_time() - start;
}

unsigned long
bench_insert_records_linear(ion_dictionary_t *dict, long count) {
    long start = ion_time();
    for (long i = 0; i < count; ++i) {
        dictionary_insert(dict, IONIZE(i, int), IONIZE(i, int));
    }
    return ion_time() - start;
}

void
benchmark_log_insert(FILE *file, long count, unsigned long time, ion_linear_hash_table_t *lht) {
    int size = snprintf(
            str,
            200,
            "%li,%li,%d,%d,%d\n",
            count,
            time,
            lht->num_records,
            lht->current_size,
            lht->total_buckets
    );
    if (size < 0 || size > 200) {
        printf("Unable to create insert log string\n");
    } else {
        fwrite(str, (size_t) size, 1, file);
    }
}

/**
 * Benchmarks inserting records building up to count total in a step size.
 *
 * Eg if count is 100 and setup is 10 then 10, 20, 30, 40 etc records will be inserted into seperate databases.
 *
 * @param count The total number of records to insert
 * @param step The steps to increment.
 */
void
benchmark_inserts(long start, long total, long step_size, ion_boolean_t remove_dict,
                  ion_dictionary_handler_t *handler) {
    ion_dictionary_t *dict;

    FILE *log = fopen("inserts.csv", "a+");

    if (NULL == log) {
        printf("Unable to open the inserts log file. Inserts will continue but will only be logged to the console.\n");
    }
    fseek(log, 0, SEEK_SET);
    char *header = "Records,Time(ms),Records in Table, Table Size, Total Buckets (incl overflow)\n";

    fread(str, strlen(header), 1, log);

    if (0 != strcmp(str, header)) {
        fwrite(header, strlen(header), 1, log);
    }

    for (long i = start; i <= total; i += step_size) {
        dict = bench_dictionary_create(handler);
        if (NULL != dict) {
            unsigned long time = bench_insert_records_random(dict, i);
            if (NULL != log) {
                benchmark_log_insert(log, i, time, (ion_linear_hash_table_t *) dict->instance);
            }
            printf("Inserted %li records in %lu ms\n", i, time);
            dictionary_delete_dictionary(dict);
            free(dict);
        } else {
            printf("Failed to create dictionary\n");
            fclose(log);
            return;
        }
    }
    fclose(log);
}

void
benchmark_log_get(FILE *file, long count, int hits, unsigned long time, ion_linear_hash_table_t *lht) {
    int size = snprintf(
            str,
            200,
            "%li,%i,%li,%d,%d,%d\n",
            count,
            hits,
            time,
            lht->num_records,
            lht->current_size,
            lht->total_buckets
    );
    if (size < 0 || size > 200) {
        printf("Unable to create insert log string\n");
    } else {
        if (NULL != file) {
            fwrite(str, (size_t) size, 1, file);
        }
    }
}

/**
 * Benchmarks inserting records building up to count total in a step size.
 *
 * Eg if count is 100 and setup is 10 then 10, 20, 30, 40 etc records will be inserted into seperate databases.
 *
 * @param count The total number of records to insert
 * @param step The steps to increment.
 */
void
benchmark_gets(long dictionary_size, long num_gets, ion_dictionary_handler_t *handler) {
    ion_dictionary_t *dict;

    FILE *log = fopen("gets.csv", "a+");

    if (NULL == log) {
        printf("Unable to open the gets log file. Gets will continue but will only be logged to the console.\n");
    }
    fseek(log, 0, SEEK_SET);
    char *header = "Number of searches,Hits,Time Total(ms),Records in Table, Table Size, Total Buckets (incl overflow)\n";

    fread(str, strlen(header), 1, log);

    if (0 != strcmp(str, header)) {
        fwrite(header, strlen(header), 1, log);
    }
    dict = bench_dictionary_create(handler);
    if (NULL != dict) {
        bench_insert_records_random(dict, dictionary_size);
        lfsr_reset(&lfsr);
        unsigned long start = ion_time();
        int hits = 0;
        int value = 0;
        ion_status_t status;
        for (long i = 0; i < num_gets; ++i) {
            int key = lfsr_get_next(&lfsr);
            status = dictionary_get(dict, IONIZE(key, int), IONIZE(value, int));
            hits += status.count;
        }
        unsigned long time = ion_time() - start;
        printf("Tried to get %li randomish records from a table with %li records in %lu (ms). (%d hits)\n", num_gets,
               dictionary_size, time, hits);
        if (NULL != log) {
            benchmark_log_get(log, num_gets, hits, time, (ion_linear_hash_table_t *) dict->instance);
        }
        dictionary_delete_dictionary(dict);
        free(dict);
    } else {
        printf("Failed to create dictionary\n");
        fclose(log);
        return;
    }
    fclose(log);
}


void
benchmark_log_deletes(long count, int hits, unsigned long time, ion_linear_hash_table_t *lht, int num_records_initial,
                      int table_size_initial, int buckets_initial) {
    FILE *log = fopen("deletes.csv", "a+");

    if (NULL == log) {
        printf("Unable to open the deletes log file\n");
        return;
    }
    fseek(log, 0, SEEK_SET);
    char *header = "Number of keys,Number deleted,Time Total(ms),Records in Table (initial),Records in Table (final), Table Size(initial),Table Size(final), Total Buckets (incl overflow)(initial), Total Buckets (incl overflow)(final)\n";

    fread(str, strlen(header), 1, log);

    if (0 != strcmp(str, header)) {
        fwrite(header, strlen(header), 1, log);
    }


    int size = snprintf(
            str,
            200,
            "%li,%i,%li,%d,%d,%d,%d,%d,%d\n",
            count,
            hits,
            time,
            num_records_initial,
            lht->num_records,
            table_size_initial,
            lht->current_size,
            buckets_initial,
            lht->total_buckets
    );
    if (size < 0 || size > 200) {
        printf("Unable to create insert log string\n");
    } else {
        if (NULL != log) {
            fwrite(str, (size_t) size, 1, log);
        }
    }
    fclose(log);
}


/**
 * Benchmarks inserting records building up to count total in a step size.
 *
 * Eg if count is 100 and setup is 10 then 10, 20, 30, 40 etc records will be inserted into seperate databases.
 *
 * @param count The total number of records to insert
 * @param step The steps to increment.
 */
void
benchmark_deletes(long dictionary_size, long num_deletes, ion_dictionary_handler_t *handler) {
    ion_dictionary_t *dict;

    dict = bench_dictionary_create(handler);
    if (NULL != dict) {
        lfsr_reset(&lfsr);
        bench_insert_records_random(dict, dictionary_size);
        lfsr_reset(&lfsr);
        ion_linear_hash_table_t *table = (ion_linear_hash_table_t *) dict->instance;
        int records_initial = table->num_records;
        int table_size = table->current_size;
        int num_buckets = table->total_buckets;
        unsigned long start = ion_time();
        int hits = 0;
        ion_status_t status;
        for (long i = 0; i < num_deletes; ++i) {
            int key = lfsr_get_next(&lfsr);
            status = dictionary_delete(dict, IONIZE(key, int));
            hits += status.count;
        }
        unsigned long time = ion_time() - start;
        printf("Tried to delete %li randomish records from a table with %li records in %lu (ms). (%d hits)\n",
               num_deletes,
               dictionary_size, time, hits);
        benchmark_log_deletes(num_deletes, hits, time, (ion_linear_hash_table_t *) dict->instance, records_initial,
                              table_size, num_buckets);
        dictionary_delete_dictionary(dict);
        free(dict);
    } else {
        printf("Failed to create dictionary\n");
        return;
    }
}

void run_benchmarks() {
    fdeleteall()
    printf("Initializing benchmarks\n");
    lfsr_init_start_state(1231, &lfsr);
    ion_dictionary_handler_t handler;
    ion_linear_hash_dict_init(&handler);

    // Testing Inserts
    benchmark_inserts(1000, max_insert, 5000, boolean_false, &handler);

    // Benchmark Gets
    benchmark_gets(1000, 500, &handler);
    benchmark_gets(5000, 2500, &handler);
    benchmark_gets(10000, 5000, &handler);
    benchmark_gets(50000, 25000, &handler);
    benchmark_gets(100000, 50000, &handler);

    // Benchmark Deletes
    benchmark_deletes(1000, 500, &handler);
    benchmark_deletes(5000, 2500, &handler);
    benchmark_deletes(10000, 5000, &handler);
    benchmark_deletes(50000, 25000, &handler);
    benchmark_deletes(100000, 50000, &handler);
    printf("Completed\n");
}
