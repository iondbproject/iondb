#include "linear_hash.c"
#include<stdio.h>
#include <stdlib.h>


void
clean_data_file(

) {
    FILE *delete_file;
    delete_file = fopen("data.bin", "w");
    fclose(delete_file);
}


int main() {
    // Clean data file
    clean_data_file();
    printf("bucket size: %lu, record size: %lu, int size: %lu\n", sizeof(linear_hash_bucket_t), sizeof(linear_hash_record_t), sizeof(int));

    printf("\nPERFORMING INITS\n");
    linear_hash_table_t* linear_hash;
    array_list_t *bucket_map = malloc(sizeof(array_list_t));
    bucket_map = array_list_init(5, bucket_map);
    bucket_map = array_list_init(5, bucket_map);
    printf("array list init'd");

    linear_hash_init(5, 50, bucket_map, linear_hash);

    printf("\nPERFORMING INSERTS\n");
    linear_hash_insert(35, insert_hash_to_bucket(35, linear_hash), linear_hash);
    linear_hash_insert(15, insert_hash_to_bucket(15, linear_hash), linear_hash);
    linear_hash_insert(5, insert_hash_to_bucket(5, linear_hash), linear_hash);
    linear_hash_insert(20, insert_hash_to_bucket(25, linear_hash), linear_hash);

    linear_hash_insert(15, insert_hash_to_bucket(15, linear_hash), linear_hash);
    linear_hash_insert(10, insert_hash_to_bucket(10, linear_hash), linear_hash);
    linear_hash_insert(15, insert_hash_to_bucket(15, linear_hash), linear_hash);
    linear_hash_insert(20, insert_hash_to_bucket(20, linear_hash), linear_hash);

    linear_hash_insert(15, insert_hash_to_bucket(15, linear_hash), linear_hash);
    linear_hash_insert(10, insert_hash_to_bucket(10, linear_hash), linear_hash);
    linear_hash_insert(15, insert_hash_to_bucket(15, linear_hash), linear_hash);


    printf("\nPRINT BUCKET 0\n");
    print_all_linear_hash_index_buckets(0, linear_hash);

    printf("\nPRINT BUCKET 5\n");
    print_all_linear_hash_index_buckets(5, linear_hash);

    return 0;
}
