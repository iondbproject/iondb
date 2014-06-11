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
#include <stdio.h>

#include "./../../kv_system.h"
#include "./../../io.h"

#define EMPTY 			-1
#define DELETED 		-2
#define IN_USE 			-3
#define SIZEOF(STATUS) 	1

/**
@brief		Options for write concern for for overwriting (updating) of values
			on insert and if not it will insert value insert_unique which
			allows for unique insert only
 */
enum write_concern
{
	wc_update,				/**< allows for values to be overwritten if already
	 	 	 	 	 	 	 	 in dictionary */
	wc_insert_unique,		/**< allows for unique inserts only
								(no overwrite) */
};

/**
@brief		Write concern for hashmap which limits insert/update of values.
 */
typedef char 			write_concern_t;

/**
@brief		The position in the hashmap.
 */
typedef int 			hash_t;

/**
@brief		Prototype declaration for hashmap
 */
typedef struct hashmap 	hashmap_t;

/**
@brief		Struct used to maintain information about size of key and value.
 */
typedef struct record
{
	int 			key_size;			/**< the size of the key in bytes */
	int 			value_size;			/**< the size of the value in bytes */
} record_t;

/**
@brief		Struct used to maintain individual records in the hashmap.
*/
typedef struct hash_bucket
{
	char 			status;			/**< the status of the bucket */
	char 			data[];			/**< the data in the bucket */
} hash_bucket_t;

/**
@brief		Struct used to maintain an instance of an in memory hashmap.
*/
struct hashmap
{
	int 			map_size;		/**<The size of the map in item capacity */
	record_t 		record;			/**<The record structure for items */
	write_concern_t write_concern;	/**<The current @p write_concern level
	 	 	 	 	 	 	 	 	 	 of the hashmap*/
	int				(* compute_hash)(hashmap_t *, ion_key_t, int);
									/**<The hashing function to be used for
										the instance*/
	char 			*entry;			/**<Pointer to the entries in the hashmap*/
};

/**
@brief		This function initializes an open address in memory hash map.

@param		hashmap
				Pointer to the hashmap instance to initialize.
@param		hashing_function
				Function pointer to the hashing function for the instance.
@param 		key_size
				The size of the key in bytes.
@param		value_size
				The size of the value in bytes.
@param		size
				The size of the hashmap in item
				(@p key_size + @p value_size + @c 1)
@return		The status describing the result of the initialization.
 */
char
oah_initialize(
		hashmap_t	*hash_map,
		hash_t		(*hashing_function)(hashmap_t *, ion_key_t, int),
		int			key_size,
		int			value_size,
		int			size
);

/**
@brief		Destroys the map in memory

@details	Destroys the map in memory and frees the underlying memory.

@param		hash_map
				The map into which the data is going to be inserted
@return		The status describing the result of the destruction
*/
err_t
oah_destroy(
		hashmap_t 	*hash_map
);

/**
@brief		Returns the theoretical location of item in hashmap

@details	Determines which bucket a record is to be placed based on the
			hash function used.

@param		num
				The key.
@param		size
				The possible number of buckets in the map.
@return		The index position to start probing at.
*/
int
oah_get_location(
		hash_t 		num,
		int 		size
);

/**
@brief		Insert record into hashmap

@details	Attempts to insert data of a given structure as dictated by record
 			into the provided hashmap.  The record is used to determine the
 			structure of the data <K,V> so that the key can be extracted.  The
 			function will return the status of the insert.  If the record has
 			been successfully inserted, the status will reflect success.  If
 			the record can not be successfully inserted the error code will
 			reflect failure.  Will only allow for insertion of unique records.

@param 		hash_map
 				The map into which the data is going to be inserted.
@param		key
 				The key that is being used to locate the position of the data.
@param		value
				The value that is being inserted.
@return 	The status of the insert.
*/
err_t
oah_insert(
		hashmap_t 		*hash_map,
		ion_key_t 		key,
		ion_value_t	 	value
);


/**
@brief		Updates a value in the map.

@details	Updates a value in the map.  If the value does not exist, it will
			insert the value.

@param		hash_map
				The map into which the data is going to be inserted.
@param		key
				The key that is being used to locate the position of the data.
@param		value
				The value that is being inserted.
@return		The status of the update
*/
err_t
oah_update(
		hashmap_t 		*hash_map,
		ion_key_t		key,
		ion_value_t 	value
);

/**
@brief 		Locates item in map.

@details	Based on a key, function locates the record in the map.

@param		hash_map
				The map into which the data is going to be inserted.
@param		record
				The structure of the record being inserted.
@param		key
				The key for the record that is being searched for.
@param		size
				The number of buckets available in the map.
@return		The index of the item in the map.
 */
int
oah_findItemLoc(
		hashmap_t 		*hash_map,
		ion_key_t	 	key
);

/**
@brief		Deletes item from map.

@details	Deletes item from map based on key.  If key does not exist
			error is returned

@param		hash_map
				The map into which the data is going to be inserted.
@param		record
				The structure of the record being inserted.
@param		key
				The key for the record that is being searched for.
@param		size
				The number of buckets available in the map.
*/
err_t
oah_delete(
		hashmap_t 		*hash_map,
		ion_key_t		key
);

/**
@brief		Locates the record if it exists.

@details	Locates the record based on key match is it exists and returns a
			pointer to the record.  This presents a significant issue as
			both the key and value could be modified, causing issues with map.

@param		hash_map
				The map into which the data is going to be inserted.
@param		record
				The structure of the record being inserted.
@param		key
				The key for the record that is being searched for.
@param		size
				The number of buckets available in the map.
@param		data
				The record <K,V> in the map.
*/
err_t
oah_query(
		hashmap_t 		*hash_map,
		ion_key_t 		key,
		char 			**data
					/** @TODO change so that memory is allocated and data copied*/
);

/**
@brief		Helper function to print out map.

@details	Helper function that displays the contents of the map including
			both key and value.

@param		hash_map
				The map into which the data is going to be inserted.
@param		size
				The number of buckets available in the map.
@param		record
				The structure of the record being inserted.
*/
void
oah_print(
		hashmap_t 	*hash_map,
		int 		size,
		record_t	*record
);

/**
@brief		A simple hashing algorithm implementation.

@param		hashmap
				The hash function is associated with.
@param		key
				The original key value to find hash value for.
@param		size_of_key
				The size of the key in bytes.
@return		The hashed value for the key.
*/
hash_t
oah_compute_simple_hash(
		hashmap_t 		*hashmap,
		ion_key_t 		key,
		int 			size_of_key
);

/*void
static_hash_init(dictonary_handler_t * client);*/

#ifdef __cplusplus
}
#endif

#endif
