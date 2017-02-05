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

    linear_hash_table_t* linear_hash;
    linear_hash_init(5, 50, linear_hash);

    printf("\nINSERTING BUCKET 0 RECORDS\n");
    linear_hash_insert(5, hash_to_bucket(5, linear_hash), linear_hash);
    linear_hash_insert(15, hash_to_bucket(15, linear_hash), linear_hash);
    linear_hash_insert(5, hash_to_bucket(5, linear_hash), linear_hash);
    linear_hash_insert(25, hash_to_bucket(25, linear_hash), linear_hash);

    linear_hash_insert(15, hash_to_bucket(15, linear_hash), linear_hash);
    linear_hash_insert(10, hash_to_bucket(10, linear_hash), linear_hash);
    linear_hash_insert(15, hash_to_bucket(15, linear_hash), linear_hash);
    linear_hash_insert(20, hash_to_bucket(20, linear_hash), linear_hash);

    linear_hash_insert(15, hash_to_bucket(15, linear_hash), linear_hash);
    linear_hash_insert(10, hash_to_bucket(10, linear_hash), linear_hash);
    linear_hash_insert(15, hash_to_bucket(15, linear_hash), linear_hash);
    linear_hash_insert(20, hash_to_bucket(20, linear_hash), linear_hash);

    linear_hash_insert(2, hash_to_bucket(2, linear_hash), linear_hash);
    linear_hash_insert(7, hash_to_bucket(7, linear_hash), linear_hash);
    linear_hash_insert(7, hash_to_bucket(7, linear_hash), linear_hash);
    linear_hash_insert(17, hash_to_bucket(17, linear_hash), linear_hash);

    linear_hash_insert(13, hash_to_bucket(13, linear_hash), linear_hash);
    linear_hash_insert(12, hash_to_bucket(12, linear_hash), linear_hash);
    linear_hash_insert(13, hash_to_bucket(13, linear_hash), linear_hash);
    linear_hash_insert(12, hash_to_bucket(12, linear_hash), linear_hash);

    linear_hash_insert(14, hash_to_bucket(14, linear_hash), linear_hash);
    linear_hash_insert(14, hash_to_bucket(14, linear_hash), linear_hash);
    linear_hash_insert(14, hash_to_bucket(14, linear_hash), linear_hash);
    linear_hash_insert(14, hash_to_bucket(14, linear_hash), linear_hash);

    linear_hash_insert(14, hash_to_bucket(14, linear_hash), linear_hash);

//    linear_hash_insert(13, hash_to_bucket(13, linear_hash), linear_hash);
//    linear_hash_insert(12, hash_to_bucket(12, linear_hash), linear_hash);
//    linear_hash_insert(13, hash_to_bucket(13, linear_hash), linear_hash);
//    linear_hash_insert(12, hash_to_bucket(12, linear_hash), linear_hash);
    linear_hash_insert(13, hash_to_bucket(13, linear_hash), linear_hash);
    linear_hash_insert(15, hash_to_bucket(15, linear_hash), linear_hash);
    
//    linear_hash_insert(15, hash_to_bucket(15, linear_hash), linear_hash);
//    linear_hash_insert(10, hash_to_bucket(10, linear_hash), linear_hash);
//    linear_hash_insert(10, hash_to_bucket(10, linear_hash), linear_hash);

//    printf("\nGET 5\n");
//    linear_hash_get(13, linear_hash);
//    linear_hash_get(25, linear_hash);
//
//    printf("\nGET 20\n");
//    linear_hash_get(20, linear_hash);

//    printf("\nGET 10\n");
//    linear_hash_get(10, linear_hash);
//
//    linear_hash_get(13, linear_hash);
//    linear_hash_get(10, linear_hash);
//    linear_hash_get(2, linear_hash);
//    linear_hash_get(17, linear_hash);

    return 0;
}
