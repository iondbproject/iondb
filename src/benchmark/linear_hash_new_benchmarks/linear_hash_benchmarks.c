#include <stdio.h>
#include "../../file/kv_stdio_intercept.h"
#include "linear_hash_benchmarks.h"
#include "./../../tests/planck-unit/src/ion_time/ion_time.h"

#ifdef ARDUINO
uint32_t delete_get_size = 60000;
#else
uint32_t delete_get_size = 1000000;
#endif
ion_dictionary_id_t dictionary_id = 0;
uint32_t increment = 10000;
#define RUNS 5

/**
 * Defines the size of the value to insert. Will be a bytes array with the first bytes of the key int.
 */
ion_value_size_t value_size = 16;

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
    ion_err_t err = dictionary_create(
            handler,
            dict,
            dictionary_id++,
            key_type_numeric_signed,
            sizeof(int),
            value_size,
            4
    );
    printf("Rc sz: %d\n", ((ion_linear_hash_table_t *)dict->instance)->record_total_size);
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
            value_size,
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
    ion_value_t *value = (ion_value_t *) alloca(value_size);
    long start = ion_time();
    for (uint32_t i = 0; i < count; ++i) {
        key = lfsr_get_next(&lfsr);
        memcpy(value, &key, sizeof(uint16_t));
        dictionary_insert(dict, IONIZE(key, int), value);
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
benchmark_block_reads_writes(int blocks, ion_dictionary_handler_t *handler) {
    ion_dictionary_t *dict = bench_dictionary_create(handler);
    ion_linear_hash_table_t *lht = (ion_linear_hash_table_t *) dict->instance;
    ion_err_t err;
    int success = 0;
    int fail = 0;
    unsigned long start = ion_time();
    for (i i = 0; i < blocks; i++) {
        err = ion_linear_hash_write_block(lht->buffer1->block.raw, i, lht);
        if (err_ok == err) {
            success++;
        } else {
            fail++;
        }
    }
    unsigned long time = ion_time() - start;
    printf("Wrote %d blocks in %lu. (success %d, fail %d)\n", blocks, time, success, fail);

    success = 0;
    fail = 0;
    start = ion_time();
    for (int i = 0; i < blocks; i++) {
        err = ion_linear_hash_read_block(i, lht, lht->buffer1->block.raw);
        if (err_ok == err) {
            success++;
        } else {
            fail++;
        }
    }
    time = ion_time() - start;
    printf("Read %d blocks in %lu. (success %d, fail %d)\n", blocks, time, success, fail);
    dictionary_delete_dictionary(dict);
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
    char *header = "Inserts,Success,Fails,Time(ms),RecordsCount,Size,Buckets,BlksR,BlksW\n";

    fread(str, strlen(header), 1, log);

    if (0 < strcmp(header, str)) {
        fwrite(header, strlen(header), 1, log);
    }

    int size = snprintf(
            str,
            200,
            "%lu,%lu,%lu,%li,%lu,%lu,%lu,%lu,%lu\n",
            (unsigned long) count,
            (unsigned long) success,
            (unsigned long) failures,
            time,
            (unsigned long) lht->num_records,
            lht->current_size,
            lht->total_buckets,
            lht->ion_linear_hash_block_reads,
            lht->ion_linear_hash_block_writes
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
 * Eg if count is 100 and setup is 10 then 10, 20, 30, 40 etc records will be inserted into separate databases.
 *
 * @param count The total number of records to insert
 * @param step The steps to increment.
 */

void
benchmark_insert(ion_dictionary_t *dict, uint32_t num, ion_boolean_t abort_on_fail) {
    ion_linear_hash_table_t *lht = (ion_linear_hash_table_t *) dict->instance;
    lht->ion_linear_hash_block_writes = 0;
    lht->ion_linear_hash_block_reads = 0;
    printf("\nInsert: Starting to insert %lu records\n", (unsigned long) num);
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
            if (boolean_true == abort_on_fail) {
                printf("Failed with error %d\n", status.error);
                return;
            }
        }
        inserted += status.count;
    }
    unsigned long end = ion_time();
    benchmark_log_inserts(num, inserted, failures, end - start, (ion_linear_hash_table_t *) dict->instance);
    printf("\tInserted %lu records in %lu ms with %lu failures: Resulting Size %lu\n", (unsigned long) num,
           end - start,
           (unsigned long) failures, (unsigned long) ((ion_linear_hash_table_t *) dict->instance)->num_records);
    printf("\tBlock R: %lu, W: %lu\n", lht->ion_linear_hash_block_reads, lht->ion_linear_hash_block_writes);
}

void
benchmark_insert_create_new(uint32_t num, ion_boolean_t remove_dict, ion_dictionary_handler_t *handler,
                            ion_boolean_t abort_on_fail) {
    printf("\nInsert: Starting to insert %lu records with a new dict\n", (unsigned long) num);
    ion_dictionary_t *dict;
    dict = bench_dictionary_create(handler);
    if (NULL != dict) {
        benchmark_insert(dict, num, abort_on_fail);
        if (boolean_true == remove_dict) {
            dictionary_delete_dictionary(dict);
        } else {
            dictionary_close(dict);
        }
        free(dict);
    } else {
        printf("\tFailed to create dictionary\n");
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
                    "%i,%d,%lu,%lu,%lu,%lu\n",
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
benchmark_log_gets(uint32_t count, uint32_t hits, uint32_t failures, uint32_t not_found, unsigned long time,
                   ion_linear_hash_table_t *lht,
                   unsigned long insert_time) {
    FILE *log = fopen("gets.csv", "a+");

    if (NULL == log) {
        printf("\tUnable to open the gets log file\n");
        return;
    }
    fseek(log, 0, SEEK_SET);
    char *header = "Ops,Hits,Fails,NotFnd,Time(ms),RecordsCnt,TableSize,Buckets,InsertTime(ms),BlkR,BlkW\n";

    fread(str, strlen(header), 1, log);

    if (0 < strcmp(header, str)) {
        fwrite(header, strlen(header), 1, log);
    }
    int size = snprintf(
            str,
            200,
            "%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",
            (unsigned long) count,
            (unsigned long) hits,
            (unsigned long) failures,
            (unsigned long) not_found,
            time,
            (unsigned long) lht->num_records,
            lht->current_size,
            lht->total_buckets,
            insert_time,
            lht->ion_linear_hash_block_reads,
            lht->ion_linear_hash_block_writes
    );
    if (size < 0 || size > 200) {
        printf("\tUnable to create insert log string\n");
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
benchmark_get(
        ion_dictionary_t *dict,
        uint32_t num_gets,
        unsigned long insert_time
) {
    ion_linear_hash_table_t *lht = (ion_linear_hash_table_t *) dict->instance;
    lht->ion_linear_hash_block_writes = 0;
    lht->ion_linear_hash_block_reads = 0;
    printf("Starting to get %lu random keys from a hash table with %lu records\n",
           (unsigned long) num_gets,
           (unsigned long) lht->num_records
    );

    // Reset the lfsr to the start seed in order to reliably hit gets
    lfsr_init_start_state((uint16_t) rand(), &lfsr);
    unsigned long start, time;
    uint32_t hits = 0;
    uint32_t failures = 0;
    uint32_t not_found = 0;
    ion_value_t *value = alloca(value_size);
    int key;
    ion_status_t status;

    printf("\tGetting %lu records\n", (unsigned long) num_gets);

    start = ion_time();
    for (uint32_t i = 0; i < num_gets; ++i) {
        key = lfsr_get_next(&lfsr);
        status = dictionary_get(dict, IONIZE(key, int), value);
        if (err_ok == status.error) {
            hits += status.count;
        } else if (err_item_not_found == status.error) {
            not_found += 1;
        } else {
            failures += 1;
        }
    }
    time = ion_time() - start;
    printf("\tTried to get %lu randomish records from a table with %lu records in %lu (ms). (%lu hits, %lu failures, %lu not-found)\n",
           (unsigned long) num_gets,
           (unsigned long) lht->num_records,
           time,
           (unsigned long) hits,
           (unsigned long) failures,
           (unsigned long) not_found
    );
    printf("\tBlock R: %lu, W: %lu\n", lht->ion_linear_hash_block_reads, lht->ion_linear_hash_block_writes);
    benchmark_log_gets(num_gets, hits, failures, not_found, time, (ion_linear_hash_table_t *) dict->instance,
                       insert_time);
}

void
benchmark_get_create_new(uint32_t dictionary_size, uint32_t num_gets, ion_dictionary_handler_t *handler,
                         ion_boolean_t remove_dict) {
    printf("Starting to build a dictionary with %lu keys for benchmarking %lu gets\n",
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
        benchmark_get(dict, num_gets, time);
        if (boolean_true == remove_dict) {
            dictionary_delete_dictionary(dict);
        } else {
            dictionary_close(dict);
        }
        free(dict);
    } else {
        printf("\tFailed to create dictionary\n");
        return;
    }
}

/**
 * @brief Loads an existing dictionary to use for getting a specified number of gets.
 */
void
benchmark_get_load_existing(
        ion_dictionary_id_t dict_id,
        uint32_t num_gets,
        ion_dictionary_handler_t *handler,
        ion_boolean_t remove_dict
) {
    ion_dictionary_t *dict = bench_dictionary_load_existing(handler, dict_id);
    if (NULL != dict) {
        printf("Loaded dict with id %d\n", dict_id);
        benchmark_get(dict, num_gets, 0);
        if (boolean_true == remove_dict) {
            dictionary_delete_dictionary(dict);
        } else {
            dictionary_close(dict);
        }
        free(dict);
    } else {
        printf("\tFailed to create dictionary\n");
        return;
    }
}


void
benchmark_log_deletes(
        uint32_t count,
        uint32_t hits,
        uint32_t fails,
        uint32_t not_found,
        unsigned long time,
        ion_linear_hash_table_t *lht,
        uint32_t num_records_initial,
        ion_linear_hash_block_index_t table_size_initial,
        ion_linear_hash_block_index_t buckets_initial,
        unsigned long insert_time
) {
    FILE *log = fopen("deletes.csv", "a+");

    if (NULL == log) {
        printf("\tUnable to open the deletes log file\n");
        return;
    }
    fseek(log, 0, SEEK_SET);
    char *header = "Ops,Hits,Fails,NotFnd,Time,Records(I),Records(F),Size(I),Size(F),Buckets(I),Buckets(F),InsertTime,BlkR,BlkW\n";

    fread(str, strlen(header), 1, log);

    if (0 < strcmp(str, header)) {
        fwrite(header, strlen(header), 1, log);
    }

    int size = snprintf(
            str,
            200,
            "%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",
            (unsigned long) count,
            (unsigned long) hits,
            (unsigned long) fails,
            (unsigned long) not_found,
            time,
            (unsigned long) num_records_initial,
            (unsigned long) lht->num_records,
            table_size_initial,
            lht->current_size,
            buckets_initial,
            lht->total_buckets,
            insert_time,
            lht->ion_linear_hash_block_reads,
            lht->ion_linear_hash_block_writes
    );
    if (size < 0 || size > 200) {
        printf("\tUnable to create insert log string\n");
    } else {
        if (NULL != log) {
            fwrite(str, (size_t) size, 1, log);
        }
    }
    fclose(log);
}

/**
 * @brief Benchmarks deleting a specified number of random records from a given dictionary.
 *
 * Note that this does not close or free the dictionary
 */
void
benchmark_delete(ion_dictionary_t *dict, uint32_t num_deletes, unsigned long insert_time) {
    ion_linear_hash_table_t *table = (ion_linear_hash_table_t *) dict->instance;
    table->ion_linear_hash_block_reads = 0;
    table->ion_linear_hash_block_writes = 0;
    printf("Starting to delete %lu random keys from a hash table with %lu records\n", (unsigned long) num_deletes,
           (unsigned long) table->num_records);

    // Reset the lfsr in order to try and get random values
    lfsr_init_start_state((uint16_t) rand(), &lfsr);

    // Log values
    uint32_t records_initial = table->num_records;
    ion_linear_hash_block_index_t table_size = table->current_size;
    ion_linear_hash_block_index_t num_buckets = table->total_buckets;
    ion_status_t status;
    int key;
    uint32_t hits = 0;
    uint32_t fails = 0;
    uint32_t not_found = 0;
    unsigned long time;

    printf("\tDeleting %lu records\n", (unsigned long) num_deletes);
    unsigned long start = ion_time();
    for (uint32_t i = 0; i < num_deletes; ++i) {
        key = lfsr_get_next(&lfsr);
        status = dictionary_delete(dict, IONIZE(key, int));
        if (err_ok == status.error) {
            hits += status.count;
        } else if (err_item_not_found == status.error) {
            not_found += 1;
        } else {
            fails += 1;
        }
    }
    time = ion_time() - start;

    // Log results
    printf("\tCompleted deleting %lu random records from a table with %lu records in %lu (ms). (%lu hits, %lu fails, %lu not-found)\n",
           (unsigned long) num_deletes,
           (unsigned long) records_initial,
           time,
           (unsigned long) hits,
           (unsigned long) fails,
           (unsigned long) not_found
    );
    printf("\tBlock R: %lu, W: %lu\n", table->ion_linear_hash_block_reads, table->ion_linear_hash_block_writes);
    benchmark_log_deletes(
            num_deletes,
            hits,
            fails,
            not_found,
            time,
            table,
            records_initial,
            table_size,
            num_buckets,
            insert_time
    );
}


void
benchmark_delete_create_new(
        uint32_t dictionary_size,
        uint32_t num_deletes,
        ion_boolean_t delete_dict,
        ion_dictionary_handler_t *handler
) {
    printf("\nDelete: Starting benchmarking deleting %lu records from an new dictionary with %lu records",
           (unsigned long) num_deletes,
           (unsigned long) dictionary_size
    );

    printf("\tStarting to create a dictionary with %lu records", (unsigned long) dictionary_size);

    unsigned long start;
    ion_dictionary_t *dict;
    dict = bench_dictionary_create(handler);

    if (NULL != dict) {
        lfsr_init_start_state((uint16_t) rand(), &lfsr);
        printf("\tInserting %d records\n", dictionary_size);
        start = ion_time();
        bench_insert_records_random(dict, dictionary_size);
        unsigned long insert_time = ion_time() - start;
        printf("\tInserted in %lu ms\n", insert_time);
        benchmark_delete(dict, num_deletes, insert_time);
        if (boolean_false == delete_dict) {
            dictionary_close(dict);
        } else {
            dictionary_delete_dictionary(dict);
        }
        free(dict);
    } else {
        printf("\tFailed to create dictionary\n");
    }
}

void
benchmark_delete_load_existing(ion_dictionary_id_t id, uint32_t num_deletes, ion_boolean_t delete_dict,
                               ion_dictionary_handler_t *handler) {
    printf("\nDelete: Starting benchmarking deleting %lu records from an existing dictionary",
           (unsigned long) num_deletes);
    printf("\tLoading dictionary with id %d\n", id);

    ion_dictionary_t *dict = bench_dictionary_load_existing(handler, id);

    if (NULL == dict) {
        printf("\tError: Unable to load the existing dictionary\n");
        return;
    }
    benchmark_delete(dict, num_deletes, 0);

    if (boolean_false == delete_dict) {
        dictionary_close(dict);
    } else {
        dictionary_delete_dictionary(dict);
    }
    free(dict);
}

void run_benchmarks() {
    fdeleteall();
    unsigned long start;
    unsigned long times[RUNS];

    ion_dictionary_handler_t handler;
    ion_linear_hash_dict_init(&handler);
//    for (int run = 0; run < RUNS; run++) {
//        benchmark_block_reads_writes(1000, &handler);
//    }

    for (int run = 0; run < RUNS; run++) {

        start = ion_time();
        printf("Initializing benchmarks with value size %d\n", value_size);
        lfsr_init_start_state((uint16_t) ion_time(), &lfsr);

//        benchmark_insert_create_new(60000, boolean_false, &handler, boolean_true);

        // Testing some small inserts
        benchmark_insert_create_new(100, boolean_true, &handler, boolean_true);
        benchmark_insert_create_new(500, boolean_true, &handler, boolean_true);
        benchmark_insert_create_new(1000, boolean_true, &handler, boolean_true);
        benchmark_insert_create_new(2000, boolean_true, &handler, boolean_true);
        benchmark_insert_create_new(3000, boolean_true, &handler, boolean_true);
        benchmark_insert_create_new(4000, boolean_true, &handler, boolean_true);
        benchmark_insert_create_new(5000, boolean_true, &handler, boolean_true);

        ion_dictionary_id_t id;
        ion_dictionary_t *dict;
//        dictionary_id = 405;
//        dict = bench_dictionary_load_existing(&handler, id);
//        if (NULL == dict) {
//            printf("Unable to load dict with id %d\n", id);
//        } else {
//            ion_linear_hash_table_t *table = (ion_linear_hash_table_t *) dict->instance;
////            benchmark_insert(dict, 1);
//            ion_err_t err = ion_linear_hash_split(table);
//            printf("Split with error %d\n", err);
//            dictionary_close(dict);
//        }
//        free(dict);
//
        for (uint32_t i = increment; i < delete_get_size; i += increment) {
            id = dictionary_id;
            dict = bench_dictionary_create(&handler);
            if (NULL == dict) {
                printf("Unable to create dict with id %d\n", id);
                continue;
            }
            // Benchmark inserting i items
            benchmark_insert(dict, i, boolean_true);

            // Benchmark getting half of the items from this dict
            printf("\n");
            benchmark_get(dict, i / 2, 0);

            // Benchmark deleting half of the items from this dict (saves us from having to create a dict later)
            printf("\n");
            benchmark_delete(dict, i / 2, 0);

            dictionary_delete_dictionary(dict);
            free(dict);
        }

        // Standard dict for common gets
        id = dictionary_id;
        dict = bench_dictionary_create(&handler);

        if (NULL == dict) {
            printf("Unable to create dict with id %d\n", id);
        } else {
            benchmark_insert(dict, delete_get_size, boolean_true);

            // Benchmark Gets with the max size dictionary
            for (uint32_t i = increment; i <= delete_get_size; i += increment) {
                printf("\n");
                benchmark_get(dict, i, 0);
            }
            // Final delete from full dict
            benchmark_delete(dict, delete_get_size / 2, 0);

            dictionary_delete_dictionary(dict);
            free(dict);
        }

////         Benchmark Deletes in increments until trying to delete the maximum records.
//        for (uint32_t i = increment; i < delete_get_size; i += increment) {
//            benchmark_delete_create_new(delete_get_size, i, boolean_true, &handler);
//        }

        times[run] = ion_time() - start;
        printf("Completed benchmarks in %lu ms\n", times[run]);
        value_size *= 2;
    }

    printf("Completed all benchmarks:\n");
    for (int i = 0; i < RUNS; i++) {
        printf("Run %d in %lu ms\n", i, times[i]);
    }
}
