/******************************************************************************/
/**
@file
@author		Scott Ronald Fazackerley
@brief		A hash table using linear probing. Designed for in memory use.
*/
/******************************************************************************/

#if !defined(OPEN_ADDRESS_FILE_H_)
#define OPEN_ADDRESS_FILE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <string.h>
#include <stdio.h>

#include "../dictionary_types.h"
#include "./../dictionary.h"
#include "open_address_file_hash_dictionary.h"

#include "../../key_value/kv_system.h"

/*edefines file operations for arduino */
#include "./../../file/SD_stdio_c_iface.h"

#define ION_EMPTY	-1
#define ION_DELETED -2
#define ION_IN_USE	-3
#define SIZEOF(STATUS) 1

/**
@brief		Prototype declaration for hashmap
*/
typedef struct file_hashmap ion_file_hashmap_t;

/**
@brief		Struct used to maintain an instance of an in memory hashmap.
*/
struct file_hashmap {
	ion_dictionary_parent_t super;
	int						map_size;	/**< The size of the map in item capacity */
	ion_write_concern_t		write_concern;	/**< The current @p write_concern level
											 of the hashmap*/

	int						(*compute_hash)(
		ion_file_hashmap_t *,
		ion_key_t,
		int
	);

	/**< The hashing function to be used for
		 the instance*/
	FILE *file;	/**< file pointer */
};

/**
@brief		This function opens a hashmap dictionary.

@param		config
				Configuration info of the desired dictionary to open.
@param		hash_map
				Pointer to the hashmap, used in subsequent calls.
@return		The status describing the result of opening the dictionary.
 */
ion_err_t
oafh_open(
	ion_dictionary_config_info_t	config,
	ion_file_hashmap_t				*hash_map
);

/**
@brief		This function closes a hashmap dictionary.

@param		hash_map
				Pointer to the hashmap instance to close.
@return		The status describing the result of closing the dictionary.
 */
ion_err_t
oafh_close(
	ion_file_hashmap_t *hash_map
);

/**
@brief		This function initializes an open address in memory hash map.

@param		hashmap
				Pointer to the hashmap instance to initialize.
@param		hashing_function
				Function pointer to the hashing function for the instance.
@param		key_type
				The type of key that is being stored in the dictionary instance.
@param		key_size
				The size of the key in bytes.
@param		value_size
				The size of the value in bytes.
@param		size
				The size of the hashmap in item
				(@p key_size + @p value_size + @c 1)
@param		id
				The id of hashmap.
@return		The status describing the result of the initialization.
*/
ion_err_t
oafh_initialize(
	ion_file_hashmap_t *hashmap,
	ion_hash_t (*hashing_function)(ion_file_hashmap_t *, ion_key_t, int),
	ion_key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	int size,
	ion_dictionary_id_t id
);

/**
@brief		Destroys the map in memory

@details	Destroys the map in memory and frees the underlying memory.

@param		hash_map
				The map into which the data is going to be inserted
@return		The status describing the result of the destruction
*/
ion_err_t
oafh_destroy(
	ion_file_hashmap_t *hash_map
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

@param	  hash_map
				The map into which the data is going to be inserted.
@param		key
				The key that is being used to locate the position of the data.
@param		value
				The value that is being inserted.
@return	 The status of the insert.
*/
ion_status_t
oafh_insert(
	ion_file_hashmap_t	*hash_map,
	ion_key_t			key,
	ion_value_t			value
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
ion_status_t
oafh_update(
	ion_file_hashmap_t	*hash_map,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief	  Locates item in map.

@details	Based on a key, function locates the record in the map.

@param		hash_map
				The map into which the data is going to be inserted.
@param		key
				The key for the record that is being searched for.
@param		location
				Pointer to the location variable
@return		The status of the find
*/
ion_err_t
oafh_find_item_loc(
	ion_file_hashmap_t	*hash_map,
	ion_key_t			key,
	int					*location
);

/**
@brief		Deletes item from map.

@details	Deletes item from map based on key.  If key does not exist
			error is returned

@param		hash_map
				The map into which the data is going to be inserted.
@param		key
				The key for the record that is being searched for.
*/
ion_status_t
oafh_delete(
	ion_file_hashmap_t	*hash_map,
	ion_key_t			key
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
ion_status_t
oafh_query(
	ion_file_hashmap_t	*hash_map,
	ion_key_t			key,
	ion_value_t			value
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
ion_hash_t
oafh_compute_simple_hash(
	ion_file_hashmap_t	*hashmap,
	ion_key_t			key,
	int					size_of_key
);

/*void
static_hash_init(ion_dictonary_handler_t * client);*/

#if defined(__cplusplus)
}
#endif

#endif
