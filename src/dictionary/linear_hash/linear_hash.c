#include "linear_hash_p.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// TODO: OPEN FILE IN INIT METHOD AND CREATE A DESTROY METHOD THAT CLOSES THE FILE
// initialization function
linear_hash_table_t*
linear_hash_init(
        int initial_size,
        int split_threshold,
        array_list_t *bucket_map,
        linear_hash_table_t *linear_hash
) {
    // open datafile
    linear_hash->database = fopen("data.bin", "r+");

    // initialize linear_hash fields
    linear_hash->initial_size = initial_size;
    linear_hash->num_buckets = initial_size;
    linear_hash->num_records = 0;
    linear_hash->next_split = 0;
    linear_hash->split_threshold = split_threshold;
    linear_hash->records_per_bucket = 4;

    // current offset pointed to in the datafile
    linear_hash->data_pointer = ftell(linear_hash->database);

    // mapping of buckets to file offsets
    linear_hash->bucket_map = bucket_map;

    // write out initial buckets
    for(int i = 0; i < linear_hash->initial_size; i++) {
        write_new_bucket(i, linear_hash);
    }

    // Save the linear_hash state to disk
    FILE *linear_hash_state;
    linear_hash_state = fopen("linear_hash_state.bin", "r+");
    // check if file is open
    if(!linear_hash_state) {
        printf("Unable to open file\n");
    }
    // write the state of the linear_hash to disk
    fwrite(linear_hash, sizeof(linear_hash_table_t), 1, linear_hash_state);
    fclose(linear_hash_state);

    printf("Linear hash table successfully initialized\n");

    //return pointer to the linear_hash that is sitting in memory
    return linear_hash;
}

linear_hash_table_t
linear_hash_read_state(

) {
    // create a pointer to the file
    FILE *linear_hash_state;
    linear_hash_state = fopen("linear_hash_state.bin", "r+");

    // create a temporary store for records that are read
    linear_hash_table_t linear_hash;

    // check if file is open
    if(!linear_hash_state) {
        printf("Unable to open file\n");
        exit(-1);
    }

    fseek(linear_hash_state, 0, SEEK_SET);
    // read state
    fread(&linear_hash, sizeof(linear_hash_table_t), 1, linear_hash_state);

    fclose(linear_hash_state);
    return linear_hash;
}

// linear hash operations
int
linear_hash_insert(
        int id,
        int hash_bucket_idx,
        linear_hash_table_t *linear_hash
) {
    // create a linear_hash_record with the desired id
    linear_hash_record_t record;
    record.id = id;

    // get the appropriate bucket for insertion
    linear_hash_bucket_t bucket = linear_hash_get_bucket(bucket_idx_to_file_offset(hash_bucket_idx, linear_hash), linear_hash);
    // location of the records in the bucket to be stored in
    file_offset bucket_loc = bucket_idx_to_file_offset(hash_bucket_idx, linear_hash);
    file_offset bucket_records_loc = get_bucket_records_location(bucket_loc);
    file_offset record_loc;

    // Case the bucket is empty
    if(bucket.anchor_record == -1) {
        record_loc = bucket_records_loc;
        bucket.anchor_record = record_loc;
        file_offset bucket_loc = bucket_idx_to_file_offset(bucket.idx, linear_hash);
        record.next = -1;
    }

    else {
        // Case that the bucket is full but there is not yet an overflow bucket
        if(linear_hash_bucket_is_full(bucket)) {
            /* Get location of overflow bucket and update the tail record for the linked list of buckets storing
             * items that hash to this bucket and update the tail bucket with the overflow's location */
            file_offset overflow_location = create_overflow_bucket(bucket.idx, linear_hash);
            printf("Bucket full, creating an overflow bucket at %ld\n", overflow_location);
            linear_hash_update_bucket(bucket_loc, bucket, linear_hash);

            /* Set the location of the anchor record on the new overflow bucket and update the record_loc for storing
             * the new record to be this location */
            file_offset overflow_anchor_record_loc = get_bucket_records_location(overflow_location);
            bucket = linear_hash_get_bucket(overflow_location, linear_hash);
            bucket.anchor_record = overflow_anchor_record_loc;
            record_loc = bucket.anchor_record;
            bucket_loc = overflow_location;
            linear_hash_update_bucket(overflow_location, bucket, linear_hash);
            record.next = -1;
        }

        // case there is >= 1 record in the bucket but it is not full
        else {
            // scan for tombstones and use if available
            file_offset tombstone_loc = bucket.anchor_record;
            linear_hash_record_t scanner = linear_hash_get_record(bucket.anchor_record);
            file_offset scanner_bucket_loc = bucket_loc;
            while(scanner.id != -1) {
                if(scanner.next == -1) {
                    // check in next overflow if there is one
                    if(bucket.overflow_location != -1) {
                        scanner_bucket_loc = bucket.overflow_location;
                        bucket = linear_hash_get_bucket(bucket.overflow_location, linear_hash);
                        tombstone_loc = bucket.anchor_record;
                        scanner = linear_hash_get_record(bucket.anchor_record);
                        printf("Scanning next overflow bucket at offset %ld, anchor record id is %d\n", scanner_bucket_loc, scanner.id);
                        continue;
                    }
                        // case there are no more overflow buckets to check in
                    else {
                        break;
                    }
                }
                tombstone_loc = scanner.next;
                scanner = linear_hash_get_record(scanner.next);
            }

            if(scanner.id == -1) {
                printf("tombstone found!\n");
                record_loc = tombstone_loc;
                record.next = scanner.next;
                bucket_loc = scanner_bucket_loc;
            }

            else {
                printf("no tombstones\n");
                bucket = linear_hash_get_bucket(bucket_loc, linear_hash);


                // get tail record and its location
                file_offset tail_loc = bucket.anchor_record + (bucket.record_count - 1) * sizeof(linear_hash_record_t);
                linear_hash_record_t tail = linear_hash_get_record(tail_loc);

                // get location to insert new record at
                record_loc = bucket.anchor_record + bucket.record_count * sizeof(linear_hash_record_t);

                // update tail record to point to newly inserted record
                tail.next = record_loc;
                linear_hash_write_record(tail_loc, tail, linear_hash);
                record.next = -1;
            }
        }
    }
    // write new record to the db
    linear_hash_write_record(record_loc, record, linear_hash);

    // update bucket
    bucket.record_count++;
    linear_hash_update_bucket(bucket_loc, bucket, linear_hash);

    linear_hash_increment_num_records(linear_hash);
    printf("Successfully inserted record %d at offset %ld\n", record.id, record_loc);
    return 1;
}

// linear hash operations
linear_hash_record_t
linear_hash_get(
        int id,
        linear_hash_table_t *linear_hash
) {
    // get the index of the bucket to read
    // get the index of the bucket to read
    int bucket_idx = insert_hash_to_bucket(id, linear_hash);
    if(bucket_idx < linear_hash->next_split) {
        bucket_idx = hash_to_bucket(id, linear_hash);
    }
    // get the bucket where the record would be located
    linear_hash_bucket_t bucket;
    bucket = linear_hash_get_bucket(bucket_idx_to_file_offset(bucket_idx, linear_hash), linear_hash);
    file_offset bucket_loc = bucket_idx_to_file_offset(bucket_idx, linear_hash);

    // create a temporary store for records that are read
    linear_hash_record_t record;
    record = linear_hash_get_record(bucket.anchor_record);
    file_offset record_loc = bucket.anchor_record;

    while(record.id != id) {
        //printf("Current record id: %d, current record next: %ld\n", record.id, record.next);
        if(record.next == -1) {
            // check in next overflow if there is one
            if(bucket.overflow_location != -1) {
                bucket_loc = bucket.overflow_location;
                bucket = linear_hash_get_bucket(bucket.overflow_location, linear_hash);
                record_loc = bucket.anchor_record;
                record = linear_hash_get_record(bucket.anchor_record);
                printf("Getting next overflow bucket at offset %ld, anchor record id is %d\n", bucket_loc, record.id);
                continue;
            }
                // case there are no more overflow buckets to check in
            else {
                printf("Record not found\n");
                return record;
            }
        }
        record_loc = record.next;
        record = linear_hash_get_record(record.next);
    }

    if(record.id == id) {
        // print record to console
        printf("Record %d found in bucket %d\n", record.id, bucket.idx);
        return record;
    }
    else {
        printf("Record not found\n");
        return record;
    }
}

// linear hash operations
void
linear_hash_delete(
        int id,
        linear_hash_table_t *linear_hash
) {
    // get the index of the bucket to read
    int bucket_idx = insert_hash_to_bucket(id, linear_hash);
    if(bucket_idx < linear_hash->next_split) {
        bucket_idx = hash_to_bucket(id, linear_hash);
    }

    // get the bucket where the record would be located
    file_offset bucket_loc = bucket_idx_to_file_offset(bucket_idx, linear_hash);
    linear_hash_bucket_t bucket = linear_hash_get_bucket(bucket_loc, linear_hash);

    // create a temporary store for records that are read
    linear_hash_record_t record;
    record = linear_hash_get_record(bucket.anchor_record);
    file_offset record_loc = bucket.anchor_record;

    while(record.id != id) {
        //printf("Current record id: %d, current record next: %ld\n", record.id, record.next);
        if(record.next == -1) {
            // check in next overflow if there is one
            if(bucket.overflow_location != -1) {
                bucket_loc = bucket.overflow_location;
                bucket = linear_hash_get_bucket(bucket.overflow_location, linear_hash);
                record_loc = bucket.anchor_record;
                record = linear_hash_get_record(bucket.anchor_record);
                printf("Getting next overflow bucket at offset %ld, anchor record id is %d\n", bucket_loc, record.id);
                continue;
            }
                // case there are no more overflow buckets to check in
            else {
                printf("Record not found\n");
                return;
            }
        }
        record_loc = record.next;
        record = linear_hash_get_record(record.next);
    }

    // set tombstone (currently using -1) as id to mark deleted record
    record.id = -1;
    printf("TOMBSTONE WRITTEN\n");
    linear_hash_write_record(record_loc, record, linear_hash);

    // decrement record count for bucket
    bucket.record_count--;
    linear_hash_decrement_num_records(linear_hash);
    linear_hash_update_bucket(bucket_loc, bucket, linear_hash);
}

// returns the struct representing the bucket at the specified index
linear_hash_bucket_t
linear_hash_get_bucket(
        file_offset bucket_loc,
        linear_hash_table_t *linear_hash
) {
    // create a temporary store for records that are read
    linear_hash_bucket_t bucket;

    // check if file is open
    if(!linear_hash->database) {
        printf("Unable to open file\n");
        exit(-1);
    }

    file_offset starting_file_offset = ftell(linear_hash->database);

    // seek to location of record in file
    fseek(linear_hash->database, bucket_loc, SEEK_SET);

    // read record
    fread(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database);

    printf("bucket %d read\n", bucket.idx);

    // restore data pointer to the original location
    fseek(linear_hash->database, starting_file_offset, SEEK_SET);
    linear_hash->data_pointer = starting_file_offset;

    return bucket;
}

// returns the struct representing the bucket at the specified index
linear_hash_record_t
linear_hash_get_record(
        file_offset loc
) {

    // create a pointer to the file
    FILE *database;
    database = fopen("data.bin", "r+");

    // create a temporary store for records that are read
    linear_hash_record_t record;

    // check if file is open
    if(!database) {
        printf("Unable to open file\n");
        exit(-1);
    }

    // seek to location of record in file
    fseek(database, loc, SEEK_SET);

    // read record
    fread(&record, sizeof(linear_hash_record_t), 1, database);

    fclose(database);

    return record;
}

void
write_new_bucket(
        int idx,
        linear_hash_table_t *linear_hash
) {
//    // create pointer to file
//    FILE *database;
//    database = fopen("data.bin", "r+");

    // store current file_offset in data file to enforce same file_offset after processing contract
    file_offset starting_file_offset = ftell(linear_hash->database);

    linear_hash_bucket_t bucket;
    // initialize bucket fields
    bucket.idx = idx;
    bucket.record_count = 0;
    bucket.overflow_location = -1;
    bucket.anchor_record = -1;

    // check the file is open
    if(!linear_hash->database) {
        printf("Unable to open file\n");
    }

    // seek to end of file to append new bucket
    // TODO REMOVE HARDCODED 4 AND USE LINEAR HASH FIELD
    // TODO THIS IS YOUR PROBLEM -- WRITE TO PROPER LOCATION EVERY TIME TO FIX
    file_offset bucket_loc;
    if(idx == 0) {
        fseek(linear_hash->database, 0, SEEK_SET);
        bucket_loc = 0;
    } else {
        fseek(linear_hash->database, linear_hash->records_per_bucket * sizeof(linear_hash_record_t), SEEK_END);
        bucket_loc = ftell(linear_hash->database);
    }

    // write to data file
    fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database);
    linear_hash_record_t blank = {-1, -1, -1};
    fwrite(&blank, sizeof(linear_hash_record_t), linear_hash->records_per_bucket, linear_hash->database);

    // restore data pointer to the original location
    fseek(linear_hash->database, starting_file_offset, SEEK_SET);
    linear_hash->data_pointer = starting_file_offset;

    // write bucket_loc in mapping
    store_bucket_loc_in_map(idx, bucket_loc);
    array_list_insert(idx, bucket_loc, linear_hash->bucket_map);
    printf("Successfully wrote a new bucket with index %d to the database\n", bucket.idx);
}

file_offset
create_overflow_bucket(
        int bucket_idx,
        linear_hash_table_t *linear_hash
) {
    // store current file_offset in data file to enforce same file_offset after processing contract
    file_offset starting_file_offset = ftell(linear_hash->database);

    // initialize bucket fields
    linear_hash_bucket_t bucket;
    bucket.idx = bucket_idx;
    bucket.record_count = 0;
    //bucket.overflow_location = -1;
    bucket.overflow_location = array_list_get(bucket_idx, linear_hash->bucket_map);
    bucket.anchor_record = -1;

    // check the file is open
    if(!linear_hash->database) {
        printf("Unable to open file\n");
    }

    // seek to end of file to append new bucket
    fseek(linear_hash->database, 0, SEEK_END);

    //file_offset overflow_loc = ftell(linear_hash->database);
    file_offset overflow_loc = ftell(linear_hash->database);
    array_list_insert(bucket.idx, overflow_loc, linear_hash->bucket_map);

    // write to file
    fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database);
    linear_hash_record_t blank = {-1, -1, -1};
    fwrite(&blank, sizeof(linear_hash_record_t), linear_hash->records_per_bucket, linear_hash->database);

    // restore data pointer to the original location
    fseek(linear_hash->database, starting_file_offset, SEEK_SET);
    linear_hash->data_pointer = starting_file_offset;

    printf("Successfully wrote an overflow bucket to the database at location %ld\n", array_list_get(bucket_idx, linear_hash->bucket_map));
    return overflow_loc;
}


void
linear_hash_write_record(
        file_offset record_loc,
        linear_hash_record_t record,
        linear_hash_table_t *linear_hash
) {
    // create pointer to file
//    FILE *database;
//    database = fopen("data.bin", "r+");

    // store current file_offset in data file to enforce same file_offset after processing contract
    file_offset starting_file_offset = ftell(linear_hash->database);

    // check the file is open
    if(!linear_hash->database) {
        printf("Unable to open file\n");
    }

    // seek to end of file to append new bucket
    fseek(linear_hash->database, record_loc, SEEK_SET);
    fwrite(&record, sizeof(linear_hash_record_t), 1, linear_hash->database);

    // restore data pointer to the original location
    fseek(linear_hash->database, starting_file_offset, SEEK_SET);
    linear_hash->data_pointer = starting_file_offset;

    printf("Successfully wrote a new record id %d, next %ld to the database\n", record.id, record.next);
}

void
linear_hash_update_bucket(
        file_offset bucket_loc,
        linear_hash_bucket_t bucket,
        linear_hash_table_t *linear_hash
) {
//    // create pointer to file
//    FILE *database;
//    database = fopen("data.bin", "r+");

    // store current file_offset in data file to enforce same file_offset after processing contract
    file_offset starting_file_offset = ftell(linear_hash->database);

    // check the file is openå
    if(!linear_hash->database) {
        printf("Unable to open file\n");
    }

    // seek to end of file to append new bucket
    fseek(linear_hash->database, bucket_loc, SEEK_SET);

    // write to file
    fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, linear_hash->database);

    // restore data pointer to the original location
    fseek(linear_hash->database, starting_file_offset, SEEK_SET);
    linear_hash->data_pointer = starting_file_offset;
    printf("Successfully updated bucket %d\n", bucket.idx);
}

int
hash_to_bucket(
        int id,
        linear_hash_table_t *linear_hash
) {
    // Case the record we are looking for was in a bucket that has already been split and h1 was used
//    if(id > linear_hash->initial_size) {
    return id % (2 * linear_hash->initial_size);
//    }
//
//    else {
//        return id % linear_hash->initial_size;
//    }

}

int
insert_hash_to_bucket(
        int id,
        linear_hash_table_t *linear_hash
) {
    return id % linear_hash->initial_size;
}

void
split(
        linear_hash_table_t *linear_hash
) {
    printf("\nPERFORMING SPLIT\n");
    // get bucket to split
    file_offset bucket_loc = bucket_idx_to_file_offset(linear_hash->next_split, linear_hash);
    linear_hash_bucket_t bucket = linear_hash_get_bucket(bucket_loc, linear_hash);
    printf("splitting bucket %d\n", bucket.idx);

    if(bucket.anchor_record != -1) {
        linear_hash_record_t record = linear_hash_get_record(bucket.anchor_record);
        file_offset record_loc = bucket.anchor_record;
    }

    linear_hash_record_t next_record;
    while(bucket.overflow_location != -1) {
        if(bucket.anchor_record != -1) {
            linear_hash_record_t record = linear_hash_get_record(bucket.anchor_record);

            while(record.next != -1) {
                next_record = linear_hash_get_record(record.next);
                if(record.id != -1) {
                    printf("deleting record %d\n", record.id);
                    linear_hash_delete(record.id, linear_hash);
                    print_linear_hash_record(record);
                    printf("\thash to bucket %d\n", hash_to_bucket(record.id, linear_hash));
                    linear_hash_insert(record.id, hash_to_bucket(record.id, linear_hash), linear_hash);
                }
                else {
                    printf("skipping tombstone\n");
                }
                record = next_record;
            }
            if(record.id != -1) {
                printf("deleting record %d\n", record.id);
                linear_hash_delete(record.id, linear_hash);
                linear_hash_insert(record.id, hash_to_bucket(record.id, linear_hash), linear_hash);
            }
            else {
                printf("skipping tombstone\n");
            }
        }

        bucket = linear_hash_get_bucket(bucket.overflow_location, linear_hash);
    }

    if(bucket.anchor_record != -1) {
        linear_hash_record_t record = linear_hash_get_record(bucket.anchor_record);
        while(record.next != -1) {
            next_record = linear_hash_get_record(record.next);
            if(record.id != -1) {
                printf("deleting record %d\n", record.id);
                linear_hash_delete(record.id, linear_hash);
                linear_hash_insert(record.id, hash_to_bucket(record.id, linear_hash), linear_hash);
            }
            else {
                printf("skipping tombstone\n");
            }
            record = next_record;
        }
        if(record.id != -1) {
            printf("deleting record %d\n", record.id);
            linear_hash_delete(record.id, linear_hash);
            linear_hash_insert(record.id, hash_to_bucket(record.id, linear_hash), linear_hash);
        }
        else {
            printf("skipping tombstone\n");
        }
    }

    linear_hash_increment_next_split(linear_hash);
    printf("split complete\n");
}

int
linear_hash_above_threshold(
        linear_hash_table_t *linear_hash
) {
    print_linear_hash_state(linear_hash);
    printf("Checking if %d records is above split threshold of %d\n", linear_hash->num_records, linear_hash->split_threshold);


    double numerator = (double)(100 * (linear_hash->num_records));
    double denominator = (double) (linear_hash->num_buckets * linear_hash->records_per_bucket);

    double load = numerator / denominator;
    printf("CURRENT LOAD %f\n", load);
    int above_threshold = (load > linear_hash->split_threshold);
    return above_threshold;
}

file_offset
get_bucket_records_location(
        file_offset bucket_loc
) {
    return bucket_loc + sizeof(linear_hash_bucket_t);
}

// Returns the file offset where bucket with index idx begins
file_offset
bucket_idx_to_file_offset(
        int idx,
        linear_hash_table_t *linear_hash
) {
//    if(idx == 0) {
//        return 0;
//    }
//    else {
//
//        // create a pointer to the file
//        FILE *linear_hash_state;
//        linear_hash_state = fopen("linear_hash_state.bin", "r+");
//
//        // seek to the location of the bucket in the map
//        file_offset loc_in_map = sizeof(linear_hash_table_t) + idx * sizeof(file_offset);
//        fseek(linear_hash_state, loc_in_map, SEEK_SET);
//
//        // read file_offset of bucket from mapping in linear hash
//        file_offset bucket_loc;
//        fread(&bucket_loc, sizeof(file_offset), 1, linear_hash_state);
//        fclose(linear_hash_state);
//        return bucket_loc;
//    }

    return array_list_get(idx, linear_hash->bucket_map);
}

// Write the offset of bucket idx to the map in linear hash state
void
store_bucket_loc_in_map(
        int idx,
        file_offset bucket_loc
) {
    // create a pointer to the file
    FILE *linear_hash_state;
    linear_hash_state = fopen("linear_hash_state.bin", "r+");

    // seek to the location of the bucket in the map
    file_offset loc_in_map = sizeof(linear_hash_table_t) + idx * sizeof(file_offset);
    fseek(linear_hash_state, loc_in_map, SEEK_SET);

    // read file_offset of bucket from mapping in linear hash
    fwrite(&bucket_loc, sizeof(file_offset), 1, linear_hash_state);
    fclose(linear_hash_state);
    printf("Wrote the location of bucket %d to %ld in map\n", idx, bucket_loc);
}

int
linear_hash_bucket_is_full(
        linear_hash_bucket_t bucket
) {
    // TODO CHANGE HARDCODED 4 TO LINEAR HASH FIELD
    return bucket.record_count == 4;
}

void
linear_hash_increment_num_records(
        linear_hash_table_t *linear_hash
) {
    linear_hash->num_records++;
    if(linear_hash_above_threshold(linear_hash)) {
        linear_hash_bucket_t *bucket;
        write_new_bucket(linear_hash->num_buckets, linear_hash);
        linear_hash_increment_num_buckets(linear_hash);
        split(linear_hash);
    }
    printf("Incremented record count to %d\n", linear_hash->num_records);
}

// decrement the count of the records stored in the linear hash
void
linear_hash_decrement_num_records(
        linear_hash_table_t *linear_hash
) {
    linear_hash->num_records--;
    printf("Decremented record count to %d\n", linear_hash->num_records);
}

void
linear_hash_increment_num_buckets(
        linear_hash_table_t *linear_hash
) {
    linear_hash->num_buckets++;
    if(linear_hash->num_buckets == 2 * linear_hash->initial_size + 1) {
        linear_hash->initial_size = linear_hash->initial_size * 2;
        printf("Size doubled, increasing intial size to %d\n", linear_hash->initial_size);
        linear_hash->next_split = 0;
    }
    printf("Incremented bucket count to %d\n", linear_hash->num_buckets);
}

void
linear_hash_increment_next_split(
        linear_hash_table_t *linear_hash
) {
    linear_hash->next_split++;
    printf("Incremented next_split to %d\n", linear_hash->next_split);
}


void
linear_hash_update_state(
        linear_hash_table_t *linear_hash
) {
    // create pointer to file
    FILE *linear_hash_state;
    linear_hash_state = fopen("linear_hash_state.bin", "r+");

    // check the file is open
    if(!linear_hash_state) {
        printf("Unable to open file\n");
    }

    // write to file
    fwrite(&linear_hash, sizeof(linear_hash_table_t), 1, linear_hash_state);
    fclose(linear_hash_state);
    printf("Updated linear hash state\n");
}

// DEBUG METHODS
void
print_linear_hash_state(
        linear_hash_table_t *linear_hash
) {
    printf("Linear Hash State\n\tinitial size: %d\n\tnum records %d\n\tnum buckets %d\n\tnext split: %d\n\tsplit threshold: %d\n",
           linear_hash->initial_size, linear_hash->num_records, linear_hash->num_buckets, linear_hash->next_split, linear_hash->split_threshold);
}

void
print_linear_hash_bucket_from_idx(
        int idx,
        linear_hash_table_t *linear_hash
) {
    linear_hash_bucket_t bucket = linear_hash_get_bucket(hash_to_bucket(idx, linear_hash), linear_hash);
    print_linear_hash_bucket(bucket);
    if(bucket.anchor_record != -1) {
        linear_hash_record_t record = linear_hash_get_record(bucket.anchor_record);
        while(record.next != -1) {
            print_linear_hash_record(record);
            record = linear_hash_get_record(record.next);
        }
        print_linear_hash_record(record);
    }
}

void
print_all_linear_hash_index_buckets(
        int idx,
        linear_hash_table_t *linear_hash
) {
    file_offset bucket_loc = bucket_idx_to_file_offset(idx, linear_hash);
    printf("PRINTING ALL %d IDX BUCKETS\nHEAD BUCKET AT %ld\n", idx, bucket_loc);
    linear_hash_bucket_t bucket = linear_hash_get_bucket(bucket_loc, linear_hash);

    while(bucket.overflow_location != -1) {
        print_linear_hash_bucket(bucket);

        if(bucket.anchor_record != -1) {
            linear_hash_record_t record = linear_hash_get_record(bucket.anchor_record);

            while(record.next != -1) {
                print_linear_hash_record(record);
                record = linear_hash_get_record(record.next);
            }

            print_linear_hash_record(record);
        }

        bucket = linear_hash_get_bucket(bucket.overflow_location, linear_hash);
    }

    if (bucket.overflow_location == -1) {
        print_linear_hash_bucket(bucket);
    }

    if(bucket.anchor_record != -1) {
        linear_hash_record_t record = linear_hash_get_record(bucket.anchor_record);
        while(record.next != -1) {
            print_linear_hash_record(record);
            record = linear_hash_get_record(record.next);
        }
        print_linear_hash_record(record);
    }
}

void
print_linear_hash_bucket(
        linear_hash_bucket_t bucket
) {
    printf("\nBucket\n\tindex %d\n\tanchor record location %ld\n\toverflow location %ld\n\trecord count: %d\n", bucket.idx, bucket.anchor_record, bucket.overflow_location, bucket.record_count);
}

void
print_linear_hash_record(
        linear_hash_record_t record
) {
    printf("\nRecord \n\tid %d\n\trecord.next location %ld\n", record.id, record.next);
}

void
print_linear_hash_bucket_map(
        linear_hash_table_t *linear_hash
) {
    printf("Bucket Map State:\n");
    for(int i = 0; i < linear_hash->bucket_map->current_size; i++) {
        printf("\tbucket idx: %d, bucket loc in data file %ld\n", i, array_list_get(i, linear_hash->bucket_map));
    }
}

array_list_t*
array_list_init(
        int init_size,
        array_list_t *array_list
) {
    array_list->current_size = init_size;
    array_list->used = 0;
    array_list->data = malloc(init_size * sizeof(file_offset));
    return array_list;
}

void
array_list_insert(
        int bucket_idx,
        file_offset bucket_loc,
        array_list_t *array_list
) {
    printf("INSERTING %ld AS HEAD LOC FOR %d\n", bucket_loc, bucket_idx);
    // case we need to expand array
    if(bucket_idx >= array_list->current_size) {
        array_list->current_size = array_list->current_size * 2;
        //array_list->current_size = array_list->current_size + 10;

        // TODO UPDATE THE POINTER TO THE ARRAY LIST ON THE LINEAR HASH AFTER REALLOC?
        array_list->data = (file_offset *) realloc(array_list->data, array_list->current_size * sizeof(file_offset));
        printf("expanded array list currrent size to %d\n", array_list->current_size);

    }

    array_list->data[bucket_idx] = bucket_loc;
}

file_offset
array_list_get(
        int bucket_idx,
        array_list_t *array_list
) {
    // case bucket_idx is outside of current size of array
    if(bucket_idx >= array_list->current_size) {
        return -1;
    }

    // case bucket_idx is inside array
    else {
        return array_list->data[bucket_idx];
    }
}

void
print_array_list_data(
        array_list_t *array_list
) {
    printf("ARRAY LIST\n\tcurrent_size = %d\n", array_list->current_size);
    for(int i = 0; i < array_list->current_size; i++) {
        printf("\t%d: %ld\n", i, array_list->data[i]);
    }
}



