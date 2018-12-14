#include <stdio.h>
#include "../../file/kv_stdio_intercept.h"
#include "linear_hash_benchmarks.h"
#include "./../../tests/planck-unit/src/ion_time/ion_time.h"

#ifdef ARDUINO
int delete_get_size = 5000;
#else
int delete_get_size = 100000;
#endif
ion_dictionary_id_t dictionary_id = 0;

char str[200];

lfsr_t lfsr;
//
//struct bucket_stats {
//    int idx;
//    int num_buckets;
//    int num_records;
//};
//
//struct bucket_stats
//bench_index_chain_stats(int idx, ion_linear_hash_table_t *table) {
//    int block = ion_array_list_get(idx, table->bucket_map);
//    int num_blocks = 0;
//    int num_records = 0;
//    ion_linear_hash_bucket_t *bucket = (ion_linear_hash_bucket_t *) table->block1;
//    ion_linear_hash_read_block(block, table, table->block1);
//    ion_boolean_t terminal = boolean_true;
//    while (terminal) {
//        num_blocks++;
//        num_records += bucket->records;
//
//        if (bucket->overflow_block == LINEAR_HASH_NO_OVERFLOW) {
//            terminal = boolean_false;
//        } else {
//            ion_linear_hash_read_block(bucket->overflow_block, table, table->block1);
//        }
//    }
//    struct bucket_stats stats;
//    stats.idx = idx;
//    stats.num_records = num_records;
//    stats.num_buckets = num_blocks;
//    return stats;
//}

ion_dictionary_t *
bench_dictionary_create(ion_dictionary_handler_t *handler) {
    ion_dictionary_t *dict = malloc(sizeof(ion_dictionary_t));
    char name[ION_MAX_FILENAME_LENGTH];
    dictionary_get_filename(dictionary_id, "lhs", name);
    fremove(name);
    dictionary_get_filename(dictionary_id, "lhd", name);
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
bench_insert_records_random(ion_dictionary_t *dict, int count) {
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
bench_insert_records_linear(ion_dictionary_t *dict, int count) {
    long start = ion_time();
    for (int i = 0; i < count; ++i) {
        dictionary_insert(dict, IONIZE(i, int), IONIZE(i, int));
    }
    return ion_time() - start;
}

void
benchmark_log_inserts(int count, int success, int failures, unsigned long time, ion_linear_hash_table_t *lht) {
    FILE *log = fopen("inserts.csv", "a+");

    if (NULL == log) {
        printf("Unable to open the inserts log file\n");
        return;
    }
    fseek(log, 0, SEEK_SET);
    char *header = "Number of insert operations,Success,Failures,Time(ms),Records in Table, Table Size, Total Buckets (incl overflow)\n";

    fread(str, strlen(header), 1, log);

    if (0 != strcmp(str, header)) {
        fwrite(header, strlen(header), 1, log);
    }

    int size = snprintf(
            str,
            200,
            "%d,%d,%d,%li,%d,%d,%d\n",
            count,
            success,
            failures,
            time,
            lht->num_records,
            lht->current_size,
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
benchmark_inserts(int num, ion_boolean_t remove_dict, ion_dictionary_handler_t *handler) {
    ion_dictionary_t *dict;
    printf("Starting to insert %d records\n", num);
    dict = bench_dictionary_create(handler);
    if (NULL != dict) {
        int inserted = 0;
        int failures = 0;
        unsigned long start = ion_time();
        ion_status_t status;
        lfsr_reset(&lfsr);
        int key;
        for (int i = 0; i < num; i++) {
            key = (int) lfsr_get_next(&lfsr);
            status = dictionary_insert(dict, IONIZE(key, int), IONIZE(key, int));
            if (err_ok != status.error) {
                failures += 1;
            }
            inserted += status.count;
        }
        unsigned long end = ion_time();
        benchmark_log_inserts(num, inserted, failures, end - start, (ion_linear_hash_table_t *) dict->instance);
        printf("Inserted %d records in %lu ms with %d failures\n", num, end - start, failures);
        dictionary_close(dict);
        free(dict);
    } else {
        printf("Failed to create dictionary\n");
        return;
    }
}

void
benchmark_inserts_individual_logs(int num, ion_boolean_t remove_dict, ion_dictionary_handler_t *handler) {
    FILE *log = fopen("inserti.csv", "w+");

    if (NULL == log) {
        printf("Unable to open the inserts log file\n");
        return;
    }
    char *header = "Num,Return Status,Time(ms),Records in Table, Table Size, Total Buckets (incl overflow)\n";

    fwrite(header, strlen(header), 1, log);

    ion_dictionary_t *dict;
    printf("Starting time individual inserts with %d records\n", num);
    dict = bench_dictionary_create(handler);
    ion_linear_hash_table_t *lht = (ion_linear_hash_table_t *) dict->instance;
    if (NULL != dict) {
        int inserted = 0;
        int failures = 0;
        unsigned long start, end;
        ion_status_t status;
        lfsr_reset(&lfsr);
        int key;
        for (int i = 0; i < num; i++) {
            key = (int) lfsr_get_next(&lfsr);
            start = ion_time();
            status = dictionary_insert(dict, IONIZE(key, int), IONIZE(key, int));
            end = ion_time();
            if (err_ok != status.error) {
                failures += 1;
            }
            inserted += status.count;


            int size = snprintf(
                    str,
                    200,
                    "%d,%d,%lu,%d,%d,%d\n",
                    i,
                    status.error,
                    end - start,
                    lht->num_records,
                    lht->current_size,
                    lht->total_buckets
            );
            if (size < 0 || size > 200) {
                printf("Unable to create insert log string\n");
            } else {
                if (NULL != log) {
                    fwrite(str, (size_t) size, 1, log);
                }
            }

        }
        dictionary_close(dict);
        free(dict);
        printf("Completed timing individual inserts with %d success and %d failures\n", inserted, failures);
    }
    fclose(log);
}

void
benchmark_log_gets(int count, int hits, unsigned long time, ion_linear_hash_table_t *lht) {
    FILE *log = fopen("gets.csv", "a+");

    if (NULL == log) {
        printf("Unable to open the gets log file\n");
        return;
    }
    fseek(log, 0, SEEK_SET);
    char *header = "Number of searches,Hits,Time Total(ms),Records in Table, Table Size, Total Buckets (incl overflow)\n";

    fread(str, strlen(header), 1, log);

    if (0 != strcmp(str, header)) {
        fwrite(header, strlen(header), 1, log);
    }
    int size = snprintf(
            str,
            200,
            "%i,%i,%li,%d,%d,%d\n",
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
benchmark_gets(int dictionary_size, int num_gets, ion_dictionary_handler_t *handler) {
    printf("\nStarting to get %d random keys from a hash table with %d records\n", num_gets, dictionary_size);
    ion_dictionary_t *dict;

    dict = bench_dictionary_create(handler);
    if (NULL != dict) {
        unsigned long start, time;
        lfsr_reset(&lfsr);
        printf("\tInserting %d records\n", dictionary_size);
        start = ion_time();
        bench_insert_records_random(dict, dictionary_size);
        time = ion_time() - start;
        printf("\tInserted in %lu ms\n", time);
        lfsr_reset(&lfsr);
        int hits = 0;
        int value = 0;
        ion_status_t status;

        printf("\tGetting %d records\n", num_gets);

        start = ion_time();
        for (int i = 0; i < num_gets; ++i) {
            int key = lfsr_get_next(&lfsr);
            status = dictionary_get(dict, IONIZE(key, int), IONIZE(value, int));
            hits += status.count;
        }
        time = ion_time() - start;
        printf("Tried to get %d randomish records from a table with %d records in %lu (ms). (%d hits)\n", num_gets,
               dictionary_size, time, hits);
        benchmark_log_gets(num_gets, hits, time, (ion_linear_hash_table_t *) dict->instance);
        dictionary_close(dict);
        free(dict);
    } else {
        printf("Failed to create dictionary\n");
        return;
    }
}


void
benchmark_log_deletes(int count, int hits, unsigned long time, ion_linear_hash_table_t *lht, int num_records_initial,
                      int table_size_initial, int buckets_initial) {
    FILE *log = fopen("deletes.csv", "a+");

    if (NULL == log) {
        printf("Unable to open the deletes log file\n");
        return;
    }
    fseek(log, 0, SEEK_SET);
    char *header = "Ops,Hits,Time(ms),Records(initial),Records(final),Table Size(init),Table Size(final),Total Buckets(initial),Total Buckets(final)\n";

    fread(str, strlen(header), 1, log);

    if (0 != strcmp(str, header)) {
        fwrite(header, strlen(header), 1, log);
    }

    int size = snprintf(
            str,
            200,
            "%d,%i,%li,%d,%d,%d,%d,%d,%d\n",
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
benchmark_deletes(int dictionary_size, int num_deletes, ion_dictionary_handler_t *handler) {
    printf("\nStarting to delete %d random keys from a hash table with %d records\n", num_deletes, dictionary_size);
    unsigned long start, time;
    int records_initial;
    int table_size;
    int num_buckets;
    int hits = 0;

    ion_dictionary_t *dict;
    dict = bench_dictionary_create(handler);

    if (NULL != dict) {
        lfsr_reset(&lfsr);
        printf("\tInserting %d records\n", dictionary_size);
        start = ion_time();
        bench_insert_records_random(dict, dictionary_size);
        time = ion_time() - start;
        printf("\tInserted in %lu ms\n", time);

        // Reset the lfsr in order to try and get the same records
        lfsr_reset(&lfsr);

        ion_linear_hash_table_t *table = (ion_linear_hash_table_t *) dict->instance;

        // Log values

        records_initial = table->num_records;
        table_size = table->current_size;
        num_buckets = table->total_buckets;
        ion_status_t status;

        printf("\tDeleting %d records\n", num_deletes);

        start = ion_time();
        for (int i = 0; i < num_deletes; ++i) {
            int key = lfsr_get_next(&lfsr);
            status = dictionary_delete(dict, IONIZE(key, int));
            hits += status.count;
        }

        time = ion_time() - start;

        // Log results
        printf("Completed deleting %d random records from a table with %d records in %lu (ms). (%d hits)\n",
               num_deletes,
               dictionary_size,
               time,
               hits
        );
        benchmark_log_deletes(
                num_deletes,
                hits,
                time,
                table,
                records_initial,
                table_size,
                num_buckets
        );
        dictionary_close(dict);
        free(dict);
        return;
    } else {
        printf("Failed to create dictionary\n");
    }
}

void run_benchmarks() {
    fdeleteall();
    unsigned long start = ion_time();
    printf("Initializing benchmarks\n");
    lfsr_init_start_state((uint16_t) ion_time(), &lfsr);
    ion_dictionary_handler_t handler;
    ion_linear_hash_dict_init(&handler);

    // Testing Inserts
    benchmark_inserts(100, boolean_false, &handler);
    benchmark_inserts(500, boolean_false, &handler);
    benchmark_inserts(1000, boolean_false, &handler);
    benchmark_inserts(2000, boolean_false, &handler);
    benchmark_inserts(3000, boolean_false, &handler);
    benchmark_inserts(4000, boolean_false, &handler);
    benchmark_inserts(5000, boolean_false, &handler);
    benchmark_inserts(10000, boolean_false, &handler);
    benchmark_inserts(15000, boolean_false, &handler);

#ifndef ARDUNIO
// Insert many more elements on PC
    benchmark_inserts(50000, boolean_false, &handler);
    benchmark_inserts(100000, boolean_false, &handler);
    benchmark_inserts(500000, boolean_false, &handler);
    benchmark_inserts(1000000, boolean_false, &handler);
#endif

#ifdef ARDUINO
    benchmark_inserts_individual_logs(5000, boolean_false, &handler);
#endif

    int percent = (int) (((long) delete_get_size) * 10 / 100); // 10% of max size

    // Benchmark Gets
    for (int i = percent; i <= delete_get_size; i += percent) {
        benchmark_gets(delete_get_size, i, &handler);
    }

    for (int i = percent; i < delete_get_size; i += percent) {
        benchmark_gets(i, i / 2, &handler);
    }

    // Benchmark Deletes in increments of 10% until trying to delete the maximum records.
    for (int i = percent; i <= delete_get_size; i += percent) {
        benchmark_deletes(delete_get_size, i, &handler);
    }

    // Benchmark deleting half of the number of records for varying size of table up to
    // (but not including delete_get_size (as that was covered in the previous delete tests)
    for (int i = percent; i < delete_get_size; i += percent) {
        benchmark_deletes(i, i / 2, &handler);
    }

    printf("Completed benchmarks in %lu ms\n", ion_time() - start);
}
