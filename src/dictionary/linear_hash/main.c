#include "linear_hash.c"
#include<stdio.h>
#include <stdlib.h>

int main() {
//    FILE *database;
//    database = fopen("data.bin", "w");
//    fclose(database);

    //printf("bucket size: %lu, record size: %lu, int size: %lu\n", sizeof(linear_hash_bucket_t), sizeof(linear_hash_record_t), sizeof(int));
    printf("HERE");

    linear_hash_table_t* linear_hash;
    linear_hash_init(5, 50, linear_hash);
//    printf("\nINSERTING BUCKET 0 RECORDS\n");
//    linear_hash_insert(5, hash_to_bucket(5, linear_hash), linear_hash);
//    linear_hash_insert(5, hash_to_bucket(5, linear_hash), linear_hash);
//    linear_hash_insert(5, hash_to_bucket(5, linear_hash), linear_hash);
//    linear_hash_insert(25, hash_to_bucket(25, linear_hash), linear_hash);
//    linear_hash_insert(15, hash_to_bucket(15, linear_hash), linear_hash);
//    linear_hash_insert(10, hash_to_bucket(10, linear_hash), linear_hash);
//    linear_hash_get(10, linear_hash);
//
//    linear_hash_insert(2, hash_to_bucket(2, linear_hash), linear_hash);
//    linear_hash_insert(7, hash_to_bucket(7, linear_hash), linear_hash);
//    linear_hash_insert(7, hash_to_bucket(7, linear_hash), linear_hash);
//    linear_hash_insert(17, hash_to_bucket(17, linear_hash), linear_hash);
//    linear_hash_insert(13, hash_to_bucket(13, linear_hash), linear_hash);
//    linear_hash_insert(12, hash_to_bucket(12, linear_hash), linear_hash);
//    linear_hash_get(13, linear_hash);
//    linear_hash_get(10, linear_hash);
//    linear_hash_get(2, linear_hash);
//    linear_hash_get(17, linear_hash);

//    linear_hash_insert(5, hash_to_bucket(5));
//    linear_hash_insert(10, hash_to_bucket(10));
//    linear_hash_insert(10, hash_to_bucket(10));
//
//    printf("\nINSERTING MORE BUCKET 0 RECORDS -- BUCKET SHOULD BE FULL\n");
//    linear_hash_insert(15, hash_to_bucket(15));
//    linear_hash_insert(10, hash_to_bucket(10));
//    linear_hash_insert(20, hash_to_bucket(20));
//    linear_hash_insert(5, hash_to_bucket(5));
//
//    printf("\nINSERTING BUCKET 2 RECORDS\n");
//    linear_hash_insert(2, hash_to_bucket(2));
//    linear_hash_insert(7, hash_to_bucket(7));
//
//    printf("\nINSERTING MORE BUCKET 2 RECORDS -- SPLIT SHOULD OCCUR\n");
//    linear_hash_insert(7, hash_to_bucket(7));
//    linear_hash_insert(2, hash_to_bucket(2));
//
//    printf("\nINSERTING MORE BUCKET 2 RECORDS -- SPLIT SHOULD OCCUR\n");
//    linear_hash_insert(7, hash_to_bucket(7));
//
//    printf("\nINSERTING MORE BUCKET 2 RECORDS -- BUCKET SHOULD BE FULL\n");
//    linear_hash_insert(7, hash_to_bucket(7));
//
//    printf("\nINSERTING MORE BUCKET 2 RECORDS -- SPLIT SHOULD OCCUR\n");
//    linear_hash_insert(7, hash_to_bucket(7));
//    linear_hash_insert(12, hash_to_bucket(12));
//
//    printf("\nINSERTING BUCKET 1 RECORDS\n");
//    linear_hash_insert(1, hash_to_bucket(1));
//    linear_hash_insert(6, hash_to_bucket(6));
//    linear_hash_insert(11, hash_to_bucket(11));
//    linear_hash_insert(1, hash_to_bucket(1));
//    linear_hash_insert(16, hash_to_bucket(16));
//    linear_hash_insert(11, hash_to_bucket(11));
//
//    printf("\nINSERTING MORE RECORDS -- BUCKET SHOULD BE FULL\n");
//    linear_hash_insert(12, hash_to_bucket(12));
//    linear_hash_insert(12, hash_to_bucket(12));
//
//    printf("\nINSERTING RECORD -- BUCKET SHOULD SPLIT\n");
//    linear_hash_insert(17, hash_to_bucket(17));
//
//    printf("\nGETTING RECORD 17\n");
//    linear_hash_get(17);
//
//    printf("\nDELETING RECORD 17\n");
//    linear_hash_delete(17);
//
//    printf("\nGETTING RECORD 17 -- SHOULD FAIL\n");
//    linear_hash_get(17);
//
//    printf("\nINSERTING BUCKET 1 RECORDS\n");
//    linear_hash_insert(1, hash_to_bucket(1));
//    linear_hash_insert(6, hash_to_bucket(6));
//    linear_hash_insert(11, hash_to_bucket(11));
//    linear_hash_insert(1, hash_to_bucket(1));
//    linear_hash_insert(16, hash_to_bucket(16));
//    linear_hash_insert(11, hash_to_bucket(11));
//
//    printf("\nGETTING RECORD 16\n");
//    linear_hash_get(16);
//
//    printf("\nDELETING RECORD 16\n");
//    linear_hash_delete(16);
//
//    printf("\nGETTING RECORD 16 -- SHOULD FAIL\n");
//    linear_hash_get(16);
    return 0;
}
