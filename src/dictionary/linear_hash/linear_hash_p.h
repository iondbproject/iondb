#include <stdio.h>

typedef long file_offset;


// SIMPLE ARRAY_LIST FOR BUCKET MAP
typedef struct {
    int current_size;
    int used;
    file_offset *data;
} array_list_t;

// definition of linear hash record, with a type and pointer instance declared for later use
typedef struct
{
    int id;
    file_offset next;
    int value;
} linear_hash_record_t;

// pointer to the anchor type for a linear hash table (which is of the type lnear_hash_record)
// TODO
typedef linear_hash_record_t bucket_anchor_t;

// buckets
// TODO NEXT POINTER TO NEXT REAL BUCKET
typedef struct {
    int idx;
    int record_count;
    file_offset anchor_record;
    file_offset overflow_location;
} linear_hash_bucket_t;

// function pointer syntax: return_type (*function_name) (arg_type)
// linear hash structure definition, with a type and pointer instance declared for later use
typedef struct {
    int initial_size;
    int next_split;
    int split_threshold;
    int num_buckets;
    int num_records;
    int records_per_bucket;
    FILE *database;

    // points to the current location in the data
    file_offset data_pointer;

    // maps the location of the head of the linked list of bucekts corresponding to its index
    array_list_t *bucket_map;

} linear_hash_table_t;

// linear hash operations
linear_hash_record_t
linear_hash_get(
        int id,
        linear_hash_table_t *linear_hash
);

int
linear_hash_insert(
        int id,
        int hash_bucket_idx,
        linear_hash_table_t* linear_hash
);

void
linear_hash_delete(
        int id,
        linear_hash_table_t *linear_hash
);

int
hash_to_bucket(
        int id,
        linear_hash_table_t *linear_hash
);

// split function
void
split(
        linear_hash_table_t *linear_hash
);

linear_hash_table_t*
linear_hash_init(
        int initial_size,
        int split_threshold,
        linear_hash_table_t *linear_hash
);

void
write_new_bucket(
        int idx,
        linear_hash_table_t *linear_hash
);

// returns the struct representing the bucket at the specified index
linear_hash_bucket_t
linear_hash_get_bucket(
        file_offset bucket_loc,
        linear_hash_table_t *linear_hash
);

void
print_linear_hash_state(
        linear_hash_table_t *linear_hash
);

void
linear_hash_update_bucket(
        file_offset bucket_loc,
        linear_hash_bucket_t bucket,
        linear_hash_table_t *linear_hash
);

linear_hash_record_t
linear_hash_get_record(
        file_offset loc
);

void
linear_hash_write_record(
        file_offset record_loc,
        linear_hash_record_t record,
        linear_hash_table_t *linear_hash
);

void
linear_hash_increment_num_records(
        linear_hash_table_t *linear_hash
);

// decrement the count of the records stored in the linear hash
void
linear_hash_decrement_num_records(
        linear_hash_table_t *linear_hash
);

void
linear_hash_increment_num_buckets(
        linear_hash_table_t *linear_hash
);

void
linear_hash_update_state(
        linear_hash_table_t *linear_hash
);

file_offset
create_overflow_bucket(

);

void
linear_hash_update_overflow_bucket(
        file_offset overflow_loc,
        linear_hash_bucket_t bucket
);

file_offset
get_bucket_records_location(
        file_offset bucket_loc
);

int
linear_hash_bucket_is_full(
        linear_hash_bucket_t bucket
);

// returns the struct representing the bucket at the specified index
linear_hash_bucket_t
linear_hash_get_overflow_bucket(
        file_offset loc
);

// Returns the file offset where bucket with index idx begins
file_offset
bucket_idx_to_file_offset(
        int idx,
        linear_hash_table_t *linear_hash
);

void
linear_hash_increment_next_split(
        linear_hash_table_t *linear_hash
);

void
print_linear_hash_bucket(
        linear_hash_bucket_t bucket
);

void
print_linear_hash_bucket_map(
        linear_hash_table_t *linear_hash
);

// Write the offset of bucket idx to the map in linear hash state
void
store_bucket_loc_in_map(
        int idx,
        file_offset bucket_loc
);

// ARRAY_LIST METHODS
array_list_t*
array_list_init(
        int init_size,
        array_list_t *array_list
);

int
array_list_insert(
        int bucket_idx,
        file_offset bucket_loc,
        array_list_t *array_list
);

file_offset
array_list_get(
        int bucket_idx,
        array_list_t *array_list
);