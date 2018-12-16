#include <stdio.h>
#include "../../file/kv_stdio_intercept.h"
#include "linear_hash_benchmarks.h"
#include "./../../tests/planck-unit/src/ion_time/ion_time.h"

#ifdef ARDUINO
uint32_t delete_get_size = 100000;
#else
uint32_t delete_get_size = 1000000;
#endif
ion_dictionary_id_t dictionary_id = 0;

#define RUNS 5

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

ion_dictionary_t *
bench_dictionary_load_existing(ion_dictionary_handler_t *handler, ion_dictionary_id_t id) {
    ion_dictionary_t *dict = malloc(sizeof(ion_dictionary_t));
    ion_err_t err = dictionary_create(
            handler,
            dict,
            id,
            key_type_numeric_signed,
            sizeof(int),
            sizeof(int),
            4
    );
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
bench_insert_records_random(ion_dictionary_t *dict, uint32_t count) {
    int key;
    long start = ion_time();
    for (uint32_t i = 0; i < count; ++i) {
        key = lfsr_get_next(&lfsr);
        dictionary_insert(dict, IONIZE(key, int), IONIZE(key, int));
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
benchmark_log_inserts(uint32_t count, uint32_t success, uint32_t failures, unsigned long time,
                      ion_linear_hash_table_t *lht) {
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
            "%lu,%lu,%lu,%li,%lu,%d,%d\n",
            (unsigned long) count,
            (unsigned long) success,
            (unsigned long) failures,
            time,
            (unsigned long) lht->num_records,
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
benchmark_inserts(uint32_t num, ion_boolean_t remove_dict, ion_dictionary_handler_t *handler) {
    ion_dictionary_t *dict;
    printf("Starting to insert %lu records\n", (unsigned long) num);
    dict = bench_dictionary_create(handler);
    if (NULL != dict) {
        uint32_t inserted = 0;
        uint32_t failures = 0;
        unsigned long start = ion_time();
        ion_status_t status;
        lfsr_reset(&lfsr);
        int key;
        for (uint32_t i = 0; i < num; i++) {
            key = (int) lfsr_get_next(&lfsr);
            status = dictionary_insert(dict, IONIZE(key, int), IONIZE(key, int));
            if (err_ok != status.error) {
                failures += 1;
            }
            inserted += status.count;
        }
        unsigned long end = ion_time();
        benchmark_log_inserts(num, inserted, failures, end - start, (ion_linear_hash_table_t *) dict->instance);
        printf("Inserted %lu records in %lu ms with %lu failures: Resulting Size %lu\n", (unsigned long) num,
               end - start,
               (unsigned long) failures, (unsigned long) ((ion_linear_hash_table_t *) dict->instance)->num_records);
        if (boolean_true == remove_dict) {
            dictionary_delete_dictionary(dict);
        } else {
            dictionary_close(dict);
        }
        free(dict);
    } else {
        printf("Failed to create dictionary\n");
        return;
    }
}

void
benchmark_inserts_individual_logs(int num, ion_dictionary_handler_t *handler) {
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
                    "%i,%d,%lu,%lu,%d,%d\n",
                    i,
                    status.error,
                    end - start,
                    (unsigned long) lht->num_records,
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
benchmark_log_gets(uint32_t count, uint32_t hits, uint32_t failures, unsigned long time, ion_linear_hash_table_t *lht,
                   unsigned long insert_time) {
    FILE *log = fopen("gets.csv", "a+");

    if (NULL == log) {
        printf("Unable to open the gets log file\n");
        return;
    }
    fseek(log, 0, SEEK_SET);
    char *header = "Ops,Hits,Fails,Time(ms),RecordsCnt,TableSize,TotalBuckets,InsertTime(ms)\n";

    fread(str, strlen(header), 1, log);

    if (0 != strcmp(str, header)) {
        fwrite(header, strlen(header), 1, log);
    }
    int size = snprintf(
            str,
            200,
            "%lu,%lu,%lur,%li,%lu,%d,%d,%lu\n",
            (unsigned long) count,
            (unsigned long) hits,
            (unsigned long) failures,
            time,
            (unsigned long) lht->num_records,
            lht->current_size,
            lht->total_buckets,
            insert_time
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
 * @brief Benchmarks getting records a specified number of records from a given dictionary
 */
void
benchmark_gets_with_dict(
        ion_dictionary_t *dict,
        uint32_t num_gets,
        unsigned long insert_time
) {
    ion_linear_hash_table_t *lht = (ion_linear_hash_table_t *) dict->instance;
    printf("\nStarting to get %lu random keys from a hash table with %lu records\n",
           (unsigned long) num_gets,
           (unsigned long) lht->num_records
    );

    // Reset the lfsr to the start seed in order to reliably hit gets
    lfsr_init_start_state((uint16_t) rand(), &lfsr);
    unsigned long start, time;
    uint32_t hits = 0;
    uint32_t failures = 0;
    int value = 0;
    int key;
    ion_status_t status;

    printf("\tGetting %lu records\n", (unsigned long) num_gets);

    start = ion_time();
    for (uint32_t i = 0; i < num_gets; ++i) {
        key = lfsr_get_next(&lfsr);
        status = dictionary_get(dict, IONIZE(key, int), IONIZE(value, int));
        if (err_ok == status.error) {
            hits += status.count;
        } else {
            failures += 1;
        }
    }
    time = ion_time() - start;
    printf("Tried to get %lu randomish records from a table with %lu records in %lu (ms). (%lu hits, %lu failures)\n",
           (unsigned long) num_gets,
           (unsigned long) lht->num_records,
           time,
           (unsigned long) hits,
           (unsigned long) failures
    );
    benchmark_log_gets(num_gets, hits, 0, time, (ion_linear_hash_table_t *) dict->instance, insert_time);
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
benchmark_get_build_directory(uint32_t dictionary_size, uint32_t num_gets, ion_dictionary_handler_t *handler,
                              ion_boolean_t remove_dict) {
    printf("\nStarting to build a dictionary with %lu keys for benchmarking %lu gets\n",
           (unsigned long) dictionary_size, (unsigned long) num_gets);
    ion_dictionary_t *dict;

    dict = bench_dictionary_create(handler);
    if (NULL != dict) {
        unsigned long start, time;
        lfsr_reset(&lfsr);
        printf("\tInserting %lu records\n", (unsigned long) dictionary_size);
        start = ion_time();
        bench_insert_records_random(dict, dictionary_size);
        time = ion_time() - start;
        printf("\tInserted in %lu ms\n", time);
        benchmark_gets_with_dict(dict, num_gets, time);
        if (boolean_true == remove_dict) {
            dictionary_delete_dictionary(dict);
        } else {
            dictionary_close(dict);
        }
        free(dict);
    } else {
        printf("Failed to create dictionary\n");
        return;
    }
}

/**
 * @brief Loads an existing dictionary to use for getting a specified number of gets.
 */
void
benchmark_gets_with_existing_dict(
        ion_dictionary_id_t dict_id,
        uint32_t num_gets,
        ion_dictionary_handler_t *handler,
        ion_boolean_t remove_dict
) {
    ion_dictionary_t *dict = bench_dictionary_load_existing(handler, dict_id);
    if (NULL != dict) {
        printf("Loaded dict with id %d\n", dict_id);
        benchmark_gets_with_dict(dict, num_gets, 0);
        if (boolean_true == remove_dict) {
            dictionary_delete_dictionary(dict);
        } else {
            dictionary_close(dict);
        }
        free(dict);
    } else {
        printf("Failed to create dictionary\n");
        return;
    }
}


void
benchmark_log_deletes(uint32_t count, uint32_t hits, unsigned long time, ion_linear_hash_table_t *lht,
                      uint32_t num_records_initial,
                      int table_size_initial, int buckets_initial, unsigned long insert_time) {
    FILE *log = fopen("deletes.csv", "a+");

    if (NULL == log) {
        printf("Unable to open the deletes log file\n");
        return;
    }
    fseek(log, 0, SEEK_SET);
    char *header = "Ops,Hits,Time(ms),Records(init),Records(fin),Size(init),Size(final),Buckets(initial),Buckets(final),InsertTime\n";

    fread(str, strlen(header), 1, log);

    if (0 != strcmp(str, header)) {
        fwrite(header, strlen(header), 1, log);
    }

    int size = snprintf(
            str,
            200,
            "%d,%i,%li,%d,%lu,%d,%d,%d,%d,%lu\n",
            count,
            hits,
            time,
            num_records_initial,
            (unsigned long) lht->num_records,
            table_size_initial,
            lht->current_size,
            buckets_initial,
            lht->total_buckets,
            insert_time
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
benchmark_deletes(uint32_t dictionary_size, uint32_t num_deletes, ion_dictionary_handler_t *handler) {
    printf("\nStarting to delete %lu random keys from a hash table with %lu records\n", (unsigned long) num_deletes,
           (unsigned long) dictionary_size);
    unsigned long start, time;
    uint32_t records_initial;
    int table_size;
    int num_buckets;
    uint32_t hits = 0;

    ion_dictionary_t *dict;
    dict = bench_dictionary_create(handler);

    if (NULL != dict) {
        lfsr_init_start_state((uint16_t) rand(), &lfsr);
        printf("\tInserting %d records\n", dictionary_size);
        start = ion_time();
        bench_insert_records_random(dict, dictionary_size);
        unsigned long insert_time = ion_time() - start;
        printf("\tInserted in %lu ms\n", insert_time);

        // Reset the lfsr in order to try and get random values
        lfsr_init_start_state((uint16_t) rand(), &lfsr);

        ion_linear_hash_table_t *table = (ion_linear_hash_table_t *) dict->instance;

        // Log values

        records_initial = table->num_records;
        table_size = table->current_size;
        num_buckets = table->total_buckets;
        ion_status_t status;
        int key;

        printf("\tDeleting %lu records\n", (unsigned long) num_deletes);
        start = ion_time();
        for (uint32_t i = 0; i < num_deletes; ++i) {
            key = lfsr_get_next(&lfsr);
            status = dictionary_delete(dict, IONIZE(key, int));
            hits += status.count;
        }

        time = ion_time() - start;

        // Log results
        printf("Completed deleting %lu random records from a table with %lu records in %lu (ms). (%d hits)\n",
               (unsigned long) num_deletes,
               (unsigned long) dictionary_size,
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
                num_buckets,
                insert_time
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
    unsigned long start;
    unsigned long times[RUNS];
    for (int run = 0; run < RUNS; run++) {

        start = ion_time();
        printf("Initializing benchmarks\n");
        lfsr_init_start_state((uint16_t) ion_time(), &lfsr);
        ion_dictionary_handler_t handler;
        ion_linear_hash_dict_init(&handler);

        // Testing Inserts
        benchmark_inserts(100, boolean_true, &handler);
        benchmark_inserts(500, boolean_true, &handler);
        benchmark_inserts(1000, boolean_true, &handler);
        benchmark_inserts(2000, boolean_true, &handler);
        benchmark_inserts(3000, boolean_true, &handler);
        benchmark_inserts(4000, boolean_true, &handler);
        benchmark_inserts(5000, boolean_true, &handler);
        benchmark_inserts(10000, boolean_true, &handler);
        benchmark_inserts(15000, boolean_true, &handler);
        benchmark_inserts(100000, boolean_false, &handler);

#ifndef ARDUINO
        benchmark_inserts(1000000, boolean_false, &handler);
#endif
        ion_dictionary_id_t id = dictionary_id - 1;
        uint32_t percent = 10000;

        // Benchmark Gets with the max size dictionary
        for (uint32_t i = percent; i <= delete_get_size; i += percent) {
            benchmark_gets_with_existing_dict(id, i, &handler, boolean_false);
        }

        for (uint32_t i = percent; i < delete_get_size; i += percent) {
            benchmark_get_build_directory(i, i / 2, &handler, boolean_true);
        }

        // Benchmark Deletes in increments of 10% until trying to delete the maximum records.
        for (uint32_t i = percent; i <= delete_get_size; i += percent) {
            benchmark_deletes(delete_get_size, i, &handler);
        }

        // Benchmark deleting half of the number of records for varying size of table up to
        // (but not including delete_get_size (as that was covered in the previous delete tests)
        for (uint32_t i = percent; i < delete_get_size; i += percent) {
            benchmark_deletes(i, i / 2, &handler);
        }
        times[run] = ion_time() - start;
        printf("Completed benchmarks in %lu ms\n", times[run]);
    }
    printf("Completed all benchmarks:\n");
    for (int i = 0; i < RUNS; i++) {
        printf("Run %d in %lu ms\n", i, times[i]);
    }
}
