/******************************************************************************/
/**
@file		statichash.h
@author		Scott Ronald Fazackerley
@brief		A hash table using linear probing. Designed for in memory use.
*/
/******************************************************************************/

#ifndef STATICHASH_H
#define STATICHASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "./../../system.h"
#include "io.h"

#define EMPTY 	-1
#define DELETED -2
#define IN_USE -3

#define SIZEOF(STATUS) 1

/**
 * update: allows for overwriting (updating) of values on insert and if not it will insert value
 * insert_unique: allows for unique insert only
 */
enum write_concern
{
	wc_update,
	wc_insert_unique,			//allows
};

typedef char write_concern_t;

typedef int hash_t;

typedef struct record
{
	int key_size;
	int value_size; 
}record_t;

typedef struct hash_bucket
{
	char status;
	char data[];
} hash_bucket_t;

typedef struct hashmap
{
	int map_size;
	record_t record;
	write_concern_t write_concern;
	int(* compute_hash)(char *, int);
	char * entry;
}hashmap_t;

/**
 * @brief Initializes a linear hash map
 *
 * @details Initializes a linear hash map with a given hash function.
 * @param hash_map the map into which the data is going to be inserted
 * @param key_size the size of the key in bytes
 * @param value_size the size of the value in bytes
 * @param size the number of buckets available in the map
 * @return
 */
char
oah_initialize(
		hashmap_t * hash_map,
		hash_t (*f)(char *, int),
		int key_size,
		int value_size,
		int size);

/**
 * @brief Destroys the map in memory
 *
 * @details Destroys the map in memory and frees the underlying memory.
 *
 * @param hash_map the map into which the data is going to be inserted
 * @return
 */
err_t
oah_destroy(
		hashmap_t * hash_map);

/**
 * @brief Returns the theoretical location of item in hashmap
 *
 * @details Determines which bucket a record is to be placed based on the
 * hash function used.
 *
 * @param num the key
 * @param size the possible number of buckets in the map
 * @return the index position to start probing at
 */
int
oah_getLocation(
		hash_t num,
		int size);

/**
 * @brief Insert record into hashmap
 *
 * @details Attempts to insert data of a given structure as dictated by record into
 * the provided hashmap.  The record is used to determine the structure of
 * the data <K,V> so that the key can be extracted.  The function will return
 * the status of the insert.  If the record has been successfully inserted,
 * the status will reflect success.  If the record can not be successfully
 * inserted the error code will reflect failure.  Will only allow for insertion
 * of unique records.
 *
 * @param hash_map the map into which the data is going to be inserted
 * @param key the key that is being used to locate the position of the data
 * @param value the value that is being inserted
 * @return the status of the insert.
 */
err_t
oah_insert(
		hashmap_t * hash_map,
		char * key,
		char * value);


/**
 * @brief Updates a value in the map
 *
 * @details Updates a value in the map.  If the value does not exist, it will insert the value.
 *
 * @param hash_map the map into which the data is going to be inserted
 * @param key the key that is being used to locate the position of the data
 * @param value the value that is being inserted
 * @return the status of the update
 */
err_t
oah_update(
		hashmap_t * hash_map,
		char * key,
		char * value);

/**
 * @brief Locates item in map
 *
 * @details Based on a key, function locates the record in the map
 *
 * @param hash_map the map into which the data is going to be inserted
 * @param record the structure of the record being inserted
 * @param key the key for the record that is being searched for
 * @param size the number of buckets available in the map
 * @return the index of the item in the map
 */
int
oah_findItemLoc(
		hashmap_t * hash_map,
		char * key);

/***
 * @brief Deletes item from map
 *
 * @details Deletes item from map based on key.  If key does not exist
 * error is returned
 *
 * @param hash_map the map into which the data is going to be inserted
 * @param record the structure of the record being inserted
 * @param key the key for the record that is being searched for
 * @param size the number of buckets available in the map
 */
err_t
oah_delete(
		hashmap_t * hash_map,
		char * key);

/**
 * @brief Locates the record if it exists
 *
 * @details Locates the record based on key match is it exists and returns a
 * pointer to the record.  This presents a significant issue as both the key
 * and value could be modified, causing issues with map.
 *
 * @param hash_map the map into which the data is going to be inserted
 * @param record the structure of the record being inserted
 * @param key the key for the record that is being searched for
 * @param size the number of buckets available in the map
 * @param data the record <K,V> in the map
 */
err_t
oah_query(
		hashmap_t * hash_map,
		char * key,
		char ** data);			//TODO change so that memory is allocated and data copied

/**
 * @brief Helper function to print out map
 *
 * @details Helper function that displays the contents of the map including
 * both key and value.
 *
 * @param hash_map the map into which the data is going to be inserted
 * @param size the number of buckets available in the map
 * @param record the structure of the record being inserted
 */
void
oah_print(
		hashmap_t * hash_map,
		int size,
		record_t * record);

/**
 * @brief A simple
 * @param key
 * @param size_of_key
 * @return
 */
hash_t
oah_compute_simple_hash(
		char * key,
		int size_of_key);

/*void
static_hash_init(dictonary_handler_t * client);*/

#ifdef __cplusplus
}
#endif

#endif
