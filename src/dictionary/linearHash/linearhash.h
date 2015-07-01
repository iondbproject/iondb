/*
 * linearhash.h
 *
 *  Created on: Apr 7, 2015
 *      Author: workstation
 */

#ifndef LINEARHASH_H_
#define LINEARHASH_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "./../dicttypes.h"
#include "./../dictionary.h"
//#include "oadictionary.h"
#include "file_ll.h"

#include "./../../kv_system.h"
#include "./../../kv_io.h"


#define EMPTY 				-1
#define DELETED 			-2
#define IN_USE 				-3
#define SIZEOF(STATUS) 		1

#define RECORDS_PER_BUCKET	2				/** defines how may records will be stored in a hash bucket */

#define FREE_CACHE_MEMORY 	1
#define PRESERVE_CACHE_MEMORY 0

#define MAX_FILE_LENGTH  	20
/** @TODO The location of hash_t needs to be resolved */
/**
@brief		The position in the hashmap.
 */
typedef int 			hash_t;

/**
@brief		Prototype declaration for hashmap
 */
typedef struct linear_hashmap 	linear_hashmap_t;

/**
@brief		Struct used to maintain individual records in the hashmap.
*/
typedef struct linear_hash_bucket
{
	char 			status;			/**< the status of the bucket */
	unsigned char 	data[];			/**< the data in the bucket */
} l_hash_bucket_t;

/**
 * Stores hash values for linear hash.
 * Lower value for any given k:  h_{i-1}(k) = k mod  (2^{i-1} * L)
 * Upper valye for any given k:   h_{i}(k) = k mod  (2^{i} * L)
 *
 * where L is the number of buckets and i is the file level
 */
typedef struct hashset {
	hash_t		lower_hash;				/**< lower hash value for linear hash set*/
	hash_t		upper_hash;				/**< upper hash value for linear hash set*/
} hash_set_t;

/**
 * Actions for bound functions
 */
typedef enum action {
	action_continue,	/**< action_continue - function continues */
	action_exit     	/**< action_exit - functions exits */
} action_t;

typedef enum cache_status {
	cache_active,
	cache_flushed,
	cache_invalid
} cache_status_t;

/**
 * @brief caches page from disk
 */
typedef struct lh_page_cache {
	cache_status_t		status;		/**< status of page cache */
	int					bucket_idx;	/**< idx of page that is in cache */
	l_hash_bucket_t		*cached_bucket;
									/**< the actual data */
} lh_page_cache_t;

/**
@brief		Struct used to maintain an instance of an in memory hashmap.
*/
struct linear_hashmap
{
	dictionary_parent_t	super;
	int 				initial_map_size;
									/**< The size of the map in item capacity */
	write_concern_t 	write_concern;
									/**< The current @p write_concern level
	 	 	 	 	 	 	 	 	 	 	 of the hashmap*/
	err_t				(* compute_hash)(linear_hashmap_t *, ion_key_t, int, int, hash_set_t *);
									/**< The hashing function to be used for
										 	 the instance.
										 	 In fact it will return both possible hash.*/
/*	char 				(* compare)(ion_key_t, ion_key_t, ion_key_size_t);
										*< Comparison function for instance of map */
	char 				*entry;		/**< Pointer to the entries in the hashmap*/
	FILE				*file;		/**< file pointer */
	int					file_level;	/**< the current file level for hash */
	int 				bucket_pointer;
									/**< pointer for current bucket being spilt */
	int					id;			/**< id for files in system */
									/** @todo this could be moved to parent */
	lh_page_cache_t		cache;		/**< holds pp for cacheing */
};

/**
 * @brief 	Keeps track of the current ll file
 */
struct file_ll
{
	FILE				*ll_file;	/**< The pointer for the current file open */
	int					bucket_id;	/**< The bucket ID for the file */

};

/**
@brief		This function initializes a linearu in memory hash map.

@param		hashmap
				Pointer to the hashmap instance to initialize.
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
lh_initialize(
		linear_hashmap_t	*hashmap,
	    err_t	 			(*compute_hash)(linear_hashmap_t *, ion_key_t, int, int, hash_set_t *),
		key_type_t			key_type,
		ion_key_size_t		key_size,
		ion_value_size_t	value_size,
		int					size,
		int					id
);

/**
@brief		Destroys the map in memory

@details	Destroys the map in memory and frees the underlying memory.

@param		hash_map
				The map into which the data is going to be inserted
@return		The status describing the result of the destruction
*/
err_t
lh_destroy(
		linear_hashmap_t 	*hash_map
);


/**
 * Closes the files for the linear hashmap structure.
 *
 * @todo This needs to be addressed for other structures.
 * @param hash_map
 * @return
 */
err_t
lh_close(
	linear_hashmap_t	*hash_map
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
/*
int
lh_get_location(
		hash_t 		num,
		int 		size
);
*/

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
lh_insert(
		linear_hashmap_t 	*hash_map,
		ion_key_t 			key,
		ion_value_t	 		value
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
lh_update(
		linear_hashmap_t 	*hash_map,
		ion_key_t			key,
		ion_value_t 		value
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
/*err_t
lh_find_item_loc(
		linear_hashmap_t 	*hash_map,
		ion_key_t	 		key,
		int					*location
);*/

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
lh_delete(
		linear_hashmap_t 	*hash_map,
		ion_key_t			key
);

/**
@brief		Locates the record if it exists.

@details	Locates the record based on key match is it exists and returns a
			pointer to the record that has been materialized in memory.

@param		hash_map
				The map into which the data is going to be inserted.
@param		key
				The key for the record that is being searched for.
@param		value
				The value associated in the map.
*/
err_t
lh_query(
		linear_hashmap_t 	*hash_map,
		ion_key_t 			key,
		ion_value_t 		value
);

/**
@brief		Locates the record if it exists and established a cursor
			but requires that a predicate be established first.

@details	Locates the record based on key match is it exists and returns a
			pointer to the record that has been materialized in memory.

@param		hash_map
				The map into which the data is going to be inserted.
@param		key
				The key for the record that is being searched for.
@param		value
				The value associated in the map.
*/
err_t
lh_find(
		dictionary_t 	*dictionary,
		predicate_t 	*predicate,
		dict_cursor_t 	**cursor
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
lh_print(
		linear_hashmap_t 	*hash_map,
		int 				size,
		record_info_t		*record
);

/**
@brief		hashing function.  This assumes that the caller mallocs memory for the hash_set

			@todo efficienty of this needs to be considered as two hash values are being computed at each step.
@param		hashmap
				The hash function is associated with.
@param		key
				The original key value to find hash value for.
@param		size_of_key
				The size of the key in bytes.
@param		file_level
				Indicates which level of hash to use
@param		hash_set
				The set of hash values (split functions)
@return		The hashed value for the key.
*/
err_t
lh_compute_hash(
		linear_hashmap_t 	*hashmap,
		ion_key_t 			key,
		int 				size_of_key,
		int					file_level,
		hash_set_t			*hash_set
);

/**
 * Splits the bucket currently being pointed to
 * @param hash_map
 * @return
 */
err_t
lh_split(
	linear_hashmap_t	*hash_map
);

/*void
static_hash_init(dictonary_handler_t * client);*/

err_t
lh_search_primary_page(
    linear_hashmap_t		*hash_map,
    int						bucket_number,
    ion_key_t				key,
    ion_value_t				value
  );

err_t
lh_get_next(
    linear_hashmap_t			*hash_map,
    ll_file_t					*linked_list_file,
    ion_key_t 					key,
    ion_value_t 				value
);


int
lh_compute_bucket_number(
	  linear_hashmap_t			*hash_map,
	  hash_set_t				*hash_set
  );


action_t
lh_delete_item_action(
	linear_hashmap_t	*hash_map,
	ion_key_t			key,
	l_hash_bucket_t		*item,
	void				*num_deleted
);

err_t
lh_cache_pp(
	linear_hashmap_t	*hash_map,
	int					bucket_number
);

err_t
lh_flush_cache(
	linear_hashmap_t	*hash_map,
	int					action
);

#ifdef __cplusplus
}
#endif

#endif /* LINEARHASH_H_ */
