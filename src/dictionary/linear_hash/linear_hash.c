#include "linear_hash_p.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// TODO: OPEN FILE IN INIT METHOD AND CREATE A DESTROY METHOD THAT CLOSES THE FILE
// initialization function
void
linear_hash_init(
        int initial_size,
        int split_threshold
) {
    // create a pointer to the file
    FILE *linear_hash_state;
    linear_hash_state = fopen("linear_hash_state.bin", "r+");

    // create a temporary store for records that are read
    linear_hash_table_t linear_hash;
    linear_hash.initial_size = initial_size;
    linear_hash.num_buckets = initial_size;
    linear_hash.num_records = 0;
    linear_hash.next_split = 0;
    linear_hash.split_threshold = split_threshold;

    for(int i = 0; i < linear_hash.initial_size; i++) {
        write_new_bucket(i);
    }

    // check if file is open
    if(!linear_hash_state) {
        printf("Unable to open file\n");
    }

    // linear_hash_init
    fwrite(&linear_hash, sizeof(linear_hash_table_t), 1, linear_hash_state);

    fclose(linear_hash_state);

    printf("Linear hash table successfully initialized\n");

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
        return linear_hash;
    }

    fseek(linear_hash_state, 0, SEEK_SET);
    // read state
    fread(&linear_hash, sizeof(linear_hash_table_t), 1, linear_hash_state);

    fclose(linear_hash_state);
    return linear_hash;
}

// linear hash operations
// TODO NEED TO MAKE INSERT TAKE BOTH AN ID AND A BUCKET IDX LOCATION
int
linear_hash_insert(
        int id,
        int hash_bucket_idx
) {
    // create a linear_hash_record with the desired id
    linear_hash_record_t record;
    record.next = -1;
    record.id = id;

    // get the current state of the linear hash to determine the hash function user
    linear_hash_table_t linear_hash = linear_hash_read_state();

    // get the appropriate bucket for insertion
    linear_hash_bucket_t bucket = linear_hash_get_bucket(bucket_idx_to_file_offset(hash_bucket_idx));

    // location of the records in the bucket to be stored in
    file_offset bucket_loc = bucket_idx_to_file_offset(hash_bucket_idx);
    file_offset bucket_records_loc = get_bucket_records_location(bucket_loc);
    file_offset record_loc;

    // Case the bucket is empty
    if(bucket.anchor_record == -1) {
        record_loc = bucket_records_loc;
        bucket.anchor_record = record_loc;
        file_offset bucket_loc = bucket_idx_to_file_offset(bucket.idx);
    }

    else {
        // Get referce to last overflow bucket
        while(bucket.overflow_location != -1) {
            printf("Changing bucket to tail overflow\n");
            bucket_loc = bucket.overflow_location;
            bucket = linear_hash_get_bucket(bucket.overflow_location);
        }

        // Case that the bucket is full but there is not yet an overflow bucket
        if(linear_hash_bucket_is_full(bucket)) {
            /* Get location of overflow bucket and update the tail record for the linked list of buckets storing
             * items that hash to this bucket and update the tail bucket with the overflow's location */
            printf("Bucket full, creating an overflow bucket\n");
            file_offset overflow_location = create_overflow_bucket();
            printf("GOT %ld AS NEW OVERFLOW LOCATION\n", overflow_location);
            bucket.overflow_location = overflow_location;
            linear_hash_update_bucket(bucket_loc, bucket);

            /* Set the location of the anchor record on the new overflow bucket and update the record_loc for storing
             * the new record to be this location */
            file_offset overflow_anchor_record_loc = get_bucket_records_location(overflow_location);
            bucket = linear_hash_get_bucket(overflow_location);
            bucket.anchor_record = overflow_anchor_record_loc;
            record_loc = bucket.anchor_record;
            bucket_loc = overflow_location;
            linear_hash_update_bucket(overflow_location, bucket);
        }

        // case there is >= 1 record in the bucket but it is not full
        else {
            // scan for tombstones and use if available
            int tombstone_found = 0;
            file_offset tombstone_loc = bucket.anchor_record;
            linear_hash_record_t scanner = linear_hash_get_record(bucket.anchor_record);
            while(scanner.next != -1) {
                printf("scanning for tombstones\n");
                if(scanner.id == -1) {
                    record_loc = tombstone_loc;
                    tombstone_found = 1;
                    break;
                }
                tombstone_loc = scanner.next;
                scanner = linear_hash_get_record(scanner.next);
            }

            if(!tombstone_found) {
                // get tail record and its location
                file_offset tail_loc = bucket.anchor_record + (bucket.record_count - 1) * sizeof(linear_hash_record_t);
                linear_hash_record_t tail = linear_hash_get_record(tail_loc);

                // get location to insert new record at
                record_loc = bucket.anchor_record + bucket.record_count * sizeof(linear_hash_record_t);

                printf("Setting %d.next to %ld\n", tail.id, record_loc);
                // update tail record to point to newly inserted record
                tail.next = record_loc;
                linear_hash_write_record(tail_loc, tail);
            }
        }
    }
    // write new record to the db
    printf("Writing %d to offset %ld\n", record.id, record_loc);
    record.next = -1;
    linear_hash_write_record(record_loc, record);

    // update bucket
    bucket.record_count++;
    linear_hash_update_bucket(bucket_loc, bucket);

    linear_hash_increment_num_records();
    printf("Successfully inserted record %d at offset %ld\n", record.id, record_loc);
    return 1;
}

// linear hash operations
linear_hash_record_t
linear_hash_get(
        int id
) {
    // get the index of the bucket to read
    int bucket_idx = hash_to_bucket(id);

    // get the bucket where the record would be located
    linear_hash_bucket_t bucket;
    bucket = linear_hash_get_bucket(bucket_idx_to_file_offset(bucket_idx));
    file_offset bucket_loc = bucket_idx_to_file_offset(bucket_idx);

    // create a temporary store for records that are read
    linear_hash_record_t record;
    record = linear_hash_get_record(bucket.anchor_record);
    file_offset record_loc = bucket.anchor_record;

    printf("Searching for record %d starting at record %d\n", id, record.id);
    while(record.next != -1) {
        if(record.id == id) {
            break;
        }
        printf("Current record %d, next %ld", record.id, record.next);
        record = linear_hash_get_record(record.next);

        // check in next overflow if there is one
        if(record.id != id && record.next == -1 && bucket.overflow_location != -1) {
            printf("Reached end of bucket\n");
            bucket_loc = bucket.overflow_location;
            bucket = linear_hash_get_bucket(bucket.overflow_location);
            record_loc = bucket.anchor_record;
            record = linear_hash_get_record(bucket.anchor_record);
            printf("Getting next overflow bucket at offset %ld, anchor record id is %d, next is %ld\n", bucket_loc, record.id, record.next);
        }
    }

    if(record.id == id) {
        // print record to console
        printf("Record %d found in bucket %d\n", record.id, bucket.idx);
        return record;
    }

    else {
        // print record to console
        printf("Record %d not found\n", id);
        return record;
    }
}

// linear hash operations
void
linear_hash_delete(
        int id
) {
    // get the index of the bucket to read
    int bucket_idx = hash_to_bucket(id);

    // get the bucket where the record would be located
    file_offset bucket_loc = bucket_idx_to_file_offset(bucket_idx);
    linear_hash_bucket_t bucket = linear_hash_get_bucket(bucket_loc);

    // create a temporary store for records that are read
    linear_hash_record_t record;
    record = linear_hash_get_record(bucket.anchor_record);
    file_offset record_loc = bucket.anchor_record;

    printf("Searching for record %d starting at record %d\n", id, record.id);
    while(record.id != id) {
        printf("Current record id: %d, current record next: %ld\n", record.id, record.next);
        if(record.next == -1) {
            // check in next overflow if there is one
            if(bucket.overflow_location != -1) {
                printf("Reached end of bucket\n");
                bucket_loc = bucket.overflow_location;
                bucket = linear_hash_get_bucket(bucket.overflow_location);
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
    linear_hash_write_record(record_loc, record);

    // decrement record count for bucket
    bucket.record_count--;
    linear_hash_update_bucket(bucket_loc, bucket);
    printf("\nFINISHED DELETE\n");
}

// returns the struct representing the bucket at the specified index
linear_hash_bucket_t
linear_hash_get_bucket(
        file_offset bucket_loc
) {
    // create a pointer to the file
    FILE *database;
    database = fopen("data.bin", "r+");

    // create a temporary store for records that are read
    linear_hash_bucket_t bucket;

    // check if file is open
    if(!database) {
        printf("Unable to open file\n");
        return bucket;
    }

    // seek to location of record in file
    fseek(database, bucket_loc, SEEK_SET);

    // read record
    fread(&bucket, sizeof(linear_hash_bucket_t), 1, database);

    printf("bucket %d read\n", bucket.idx);

    fclose(database);

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
        return record;
    }

    // seek to location of record in file
    fseek(database, loc, SEEK_SET);

    // read record
    fread(&record, sizeof(linear_hash_record_t), 1, database);
    printf("File offset read: %ld, record %d read\n", loc, record.id);

    fclose(database);

    return record;
}

void
write_new_bucket(
        int idx
) {
    // create pointer to file
    FILE *database;
    database = fopen("data.bin", "r+");

    // initialize bucket fields
    linear_hash_bucket_t bucket;
    bucket.idx = idx;
    bucket.record_count = 0;
    bucket.overflow_location = - 1;
    bucket.anchor_record = -1;

    // check the file is open
    if(!database) {
        printf("Unable to open file\n");
    }

    // seek to end of file to append new bucket
    // TODO REMOVE HARDCODED 4 AND USE LINEAR HASH FIELD
    // TODO THIS IS YOUR PROBLEM -- WRITE TO PROPER LOCATION EVERY TIME TO FIX
    if(idx == 0) {
        fseek(database, 0, SEEK_SET);
    } else {
        fseek(database, 4 * sizeof(linear_hash_record_t), SEEK_END);
    }

    // write to file
    fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, database);
    fclose(database);

    printf("Successfully wrote a new bucket with index %d to the database\n", bucket.idx);
}

file_offset
create_overflow_bucket(

) {
    // create pointer to file
    FILE *database;
    database = fopen("data.bin", "r+");

    // initialize bucket fields
    linear_hash_bucket_t bucket;
    bucket.idx = -1;
    bucket.record_count = 0;
    bucket.overflow_location = - 1;
    bucket.anchor_record = -1;

    // check the file is open
    if(!database) {
        printf("Unable to open file\n");
    }

    // seek to end of file to append new bucket
    fseek(database, 0, SEEK_END);
    file_offset overflow_loc = ftell(database);

    // write to file
    // TODO REMOVE HARDCODED 4 AND USE LINEAR HASH FIELD
    fwrite(&bucket, sizeof(linear_hash_bucket_t) + 4 * sizeof(linear_hash_record_t), 1, database);
    fclose(database);

    printf("Successfully wrote an overflow bucket to the database at location %ld\n", overflow_loc);
    return overflow_loc;
}


void
linear_hash_write_record(
        file_offset record_loc,
        linear_hash_record_t record
) {
    // create pointer to file
    FILE *database;
    database = fopen("data.bin", "r+");

    // check the file is open
    if(!database) {
        printf("Unable to open file\n");
    }

    // seek to end of file to append new bucket
    fseek(database, record_loc, SEEK_SET);
    fwrite(&record, sizeof(linear_hash_record_t), 1, database);
    fclose(database);
    printf("Successfully wrote a new record id %d, next %ld to the database\n", record.id, record.next);
}

void
linear_hash_update_bucket(
        file_offset bucket_loc,
        linear_hash_bucket_t bucket
) {
    // create pointer to file
    FILE *database;
    database = fopen("data.bin", "r+");

    // check the file is open
    if(!database) {
        printf("Unable to open file\n");
    }

    // seek to end of file to append new bucket
    fseek(database, bucket_loc, SEEK_SET);

    // write to file
    fwrite(&bucket, sizeof(linear_hash_bucket_t), 1, database);
    fclose(database);

    printf("Successfully updated bucket %d\n", bucket.idx);
}

int
hash_to_bucket(
        int id
) {
    linear_hash_table_t linear_hash = linear_hash_read_state();
    int h0 = id % linear_hash.initial_size;

    // Case the record we are looking for was in a bucket that has already been split and h1 was used
    if(h0 < linear_hash.next_split) {
        return id % (2 * linear_hash.initial_size);
    }
    return h0;
}

void
split(

) {
    printf("\nPERFORMING SPLIT\n");
    // get bucket to split
    linear_hash_table_t linear_hash = linear_hash_read_state();
    print_linear_hash_state(linear_hash);
    file_offset bucket_loc = bucket_idx_to_file_offset(linear_hash.next_split);
    linear_hash_bucket_t bucket = linear_hash_get_bucket(bucket_loc);
    printf("splitting bucket %d\n", bucket.idx);
    print_linear_hash_bucket(bucket);

    // Case split bucket is empty
    if(bucket.anchor_record == -1) {
        printf("no records to split\n");
        linear_hash_increment_next_split();
        return;
    }

    linear_hash_record_t record = linear_hash_get_record(bucket.anchor_record);
    file_offset record_loc = bucket.anchor_record;
    printf("Current record id: %d, current record next: %ld\n", record.id, record.next);

    while(record.next != -1) {
        // delete record from current location and rehash
        if(record.id == -1) {
            // tombstone case
            record = linear_hash_get_record(record.next);
            continue;
        }

        linear_hash_delete(record.id);
        linear_hash_insert(record.id, hash_to_bucket(record.id));

        // check in next overflow if there is one
        if(record.next == -1 && bucket.overflow_location != -1) {
            printf("Reached end of bucket\n");
            bucket_loc = bucket.overflow_location;
            bucket = linear_hash_get_bucket(bucket.overflow_location);
            record_loc = bucket.anchor_record;
            record = linear_hash_get_record(bucket.anchor_record);
            printf("Getting next overflow bucket at offset %ld, anchor record id is %d\n", bucket_loc, record.id);
            continue;
        }

        record_loc = record.next;
        record = linear_hash_get_record(record.next);
    }
    linear_hash_increment_next_split();
    printf("split complete\n");
}

int
linear_hash_above_threshold(

) {
    linear_hash_table_t linear_hash = linear_hash_read_state();
    print_linear_hash_state(linear_hash);
    printf("Checking if %d records is above split threshold of %d\n", linear_hash.num_records, linear_hash.split_threshold);
    double numerator = (double)(100 * (linear_hash.num_records));
    double denominator = (double) (linear_hash.num_buckets * 4);
    double load = numerator / denominator;
    printf("CURRENT LOAD %f\n", load);
    return load > linear_hash.split_threshold;
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
        int idx
) {
    if(idx == 0) {
        return 0;
    }
    else {
        // TODO CHANGE HARDCODED 4 TO LINEAR HASH FIELD
        return idx * sizeof(linear_hash_bucket_t) + 4 * sizeof(linear_hash_record_t) * idx;
    }
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

) {
    linear_hash_table_t linear_hash = linear_hash_read_state();
    linear_hash.num_records++;
    linear_hash_update_state(linear_hash);
    if(linear_hash_above_threshold()) {
        write_new_bucket(linear_hash.num_buckets);
        linear_hash_increment_num_buckets();
        split();
    }
    printf("Incremented record count to %d\n", linear_hash.num_records);
}

void
linear_hash_increment_num_buckets(

) {
    linear_hash_table_t linear_hash = linear_hash_read_state();
    linear_hash.num_buckets++;
    if(linear_hash.num_buckets == 2 * linear_hash.initial_size) {
        printf("Size doubled, increasing intial size to %d\n", linear_hash.initial_size);
        linear_hash.initial_size = linear_hash.initial_size * 2;
        linear_hash.next_split = 0;
    }
    linear_hash_update_state(linear_hash);
    printf("Incremented bucket count to %d\n", linear_hash.num_buckets);
}

void
linear_hash_increment_next_split(

) {
    linear_hash_table_t linear_hash = linear_hash_read_state();
    linear_hash.next_split++;
    linear_hash_update_state(linear_hash);
    printf("Incremented next_split to %d\n", linear_hash.next_split);
}


void
linear_hash_update_state(
        linear_hash_table_t linear_hash
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
    linear_hash = linear_hash_read_state();
    printf("Updated linear hash state\n");
}

// DEBUG METHODS
void
print_linear_hash_state(
        linear_hash_table_t linear_hash
) {
    printf("Linear Hash State\n\tinitial size: %d\n\tnum records %d\n\tnum buckets %d\n\tnext split: %d\n\tsplit threshold: %d\n",
           linear_hash.initial_size, linear_hash.num_records, linear_hash.num_buckets, linear_hash.next_split, linear_hash.split_threshold);
}

void
print_linear_hash_bucket(
        linear_hash_bucket_t bucket
) {
    printf("\nBucket\n\tindex %d\n\tanchor record location %ld\n\toverflow location %ld\n", bucket.idx, bucket.anchor_record, bucket.overflow_location);
}