/******************************************************************************/
/**
@file		statichash.h
@author		Scott Ronald Fazackerley
@brief		A hash table using linear probing. Designed for in memory use.
*/
/******************************************************************************/

#ifndef OAFHASH_H
#define OAFHASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>

#include "./../dicttypes.h"
#include "./../dictionary.h"
#include "oafdictionary.h"

#include "./../../kv_system.h"
#include "./../../kv_io.h"

/**redefines file operations for arduino */
//#include "./../../sd_if/SD_stdio_c_iface.h"

#define EMPTY 			-1
#define DELETED 		-2
#define IN_USE 			-3
#define SIZEOF(STATUS) 	1



/**
@brief		Prototype declaration for hashmap
 */
typedef struct file_hashmap 	file_hashmap_t;

/**
@brief		Struct used to maintain individual records in the hashmap.
*/
typedef struct hash_bucket
{
	char 			status;			/**< the status of the bucket */
	unsigned char 	data[];			/**< the data in the bucket */
} hash_bucket_t;

/**
@brief		Struct used to maintain an instance of an in memory hashmap.
*/
struct file_hashmap
{
	dictionary_parent_t	super;
	int 				map_size;		/**< The size of the map in item capacity */
	write_concern_t 	write_concern;	/**< The current @p write_concern level
	 	 	 	 	 	 	 	 	 	 	 of the hashmap*/
	int					(* compute_hash)(file_hashmap_t *, ion_key_t, int);
										/**< The hashing function to be used for
										 	 the instance*/
/*	char 				(* compare)(ion_key_t, ion_key_t, ion_key_size_t);
										*< Comparison function for instance of map */
	//char 				*entry;				/**< Pointer to the entries in the hashmap*/
	FILE				*file;				/**< file pointer */
};

/**
@brief		This function initializes an open address in memory hash map.

@param		hashmap
				Pointer to the hashmap instance to initialize.
@param		hashing_function
				Function pointer to the hashing function for the instance.
@param		key_type
				The type of key that is being stored in the collection.
@param 		key_size
				The size of the key in bytes.
@param		value_size
				The size of the value in bytes.
@param		size
				The size of the hashmap in item
				(@p key_size + @p value_size + @c 1)
@return		The status describing the result of the initialization.
 */
err_t
oafh_initialize(
		file_hashmap_t		*hashmap,
		hash_t				(*hashing_function)(file_hashmap_t *, ion_key_t, int),
	    key_type_t			key_type,
		ion_key_size_t		key_size,
		ion_value_size_t	value_size,
		int					size
);

/**
@brief		Destroys the map in memory

@details	Destroys the map in memory and frees the underlying memory.

@param		hash_map
				The map into which the data is going to be inserted
@return		The status describing the result of the destruction
*/
err_t
oafh_destroy(
		file_hashmap_t 	*hash_map
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
oafh_get_location(
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
oafh_insert(
		file_hashmap_t 		*hash_map,
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
oafh_update(
		file_hashmap_t 		*hash_map,
		ion_key_t		key,
		ion_value_t 	value
);

/**
@brief 		Locates item in map.

@details	Based on a key, function locates the record in the map.

@param		hash_map
				The map into which the data is going to be inserted.
@param		ion_key_t
				The key for the record that is being searched for.
@param		location
				Pointer to the location variable
@return		The status of the find
 */
err_t
oafh_find_item_loc(
		file_hashmap_t 		*hash_map,
		ion_key_t	 	key,
		int				*location
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
oafh_delete(
		file_hashmap_t 		*hash_map,
		ion_key_t		key
);

/**
@brief		Locates the record if it exists.

@details	Locates the record based on key match is it exists and returns a
			pointer to the record that has been materialized in memory.
			This presents a significant issue as both the key and value could
			be modified, causing issues with map.

@param		hash_map
				The map into which the data is going to be inserted.
@param		key
				The key for the record that is being searched for.
@param		value
				The value associated in the map.
*/
err_t
oafh_query(
		file_hashmap_t 		*hash_map,
		ion_key_t 		key,
		ion_value_t 	value
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
oafh_compute_simple_hash(
		file_hashmap_t 		*hashmap,
		ion_key_t 		key,
		int 			size_of_key
);

/*void
static_hash_init(dictonary_handler_t * client);*/


#ifdef __cplusplus
}
#endif

#endif
