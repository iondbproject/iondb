/*
 * linear_hash.h
 *
 *  Created on: Apr 7, 2015
 *	  Author: workstation
 */

#if !defined(LINEAR_HASH_H_)
#define LINEAR_HASH_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <math.h>

#include "../dictionary_types.h"
#include "./../dictionary.h"
#include "../../util/id_encoder/file_encoder.h"
#include "file_linked_list.h"

#include "./../../kv_system.h"
#include "./../../kv_io.h"

#include "./../../file/SD_stdio_c_iface.h"

#include "linear_hash_dictionary.h"

#define EMPTY_BLOCK_REQUEST		-1
#define EMPTY					-1
#define DELETED					-2
#define IN_USE					-3
#define SIZEOF(STATUS) 1

#define RECORDS_PER_BUCKET		2			/** defines how may records will be stored in a hash bucket */

#define FREE_CACHE_MEMORY		1
#define PRESERVE_CACHE_MEMORY	0

#define CACHE_SIZE				2			/** defines how many cache blocks are available */

#define SPLIT_THRESHOLD			70			/** 70/100 as split threshold as per recommendation Litwin */

#define MAX_FILE_LENGTH			20

/** @TODO The location of hash_t needs to be resolved */
/**
@brief		The position in the hashmap.
 */
typedef unsigned int hash_t;

/**
@brief		Prototype declaration for hashmap
 */
typedef struct linear_hashmap linear_hashmap_t;

/**
@brief		Struct used to maintain individual records in the hashmap.
*/
typedef struct linear_hash_bucket {
	char			status;			/**< the status of the bucket */
	unsigned char	data[];			/**< the data in the bucket */
} l_hash_bucket_t;

/**
 * Stores hash values for linear hash.
 * Lower value for any given k:  h_{i-1}(k) = k mod  (2^{i-1} * L)
 * Upper valye for any given k:   h_{i}(k) = k mod  (2^{i} * L)
 *
 * where L is the number of buckets and i is the file level
 */
typedef struct hashset {
	hash_t	lower_hash;					/**< lower hash value for linear hash set*/
	hash_t	upper_hash;					/**< upper hash value for linear hash set*/
} hash_set_t;

/**
 * Actions for bound functions
 */
typedef enum action {
	action_continue,/**< action_continue - function continues */
	action_deleted, action_flush_and_exit, action_exit	/**< action_exit - functions exits with error code */
} action_t;

/**
 * Action and error codes returned from action fp*/
typedef struct action_status {
	action_t	action;	/**< The desired action */
	err_t		err;	/**< The associated error code */
} action_status_t;

typedef enum cache_status {
	cache_active,	/**< data in the cache is live but same as on disk */
	cache_active_written,	/**< data is the cache has been changed from what was originally written to disk */
	cache_flushed,	/**< data in the cache has been flushed to disk but is still in cache */
	cache_invalid	/**< data in the cache is uncertian */
} cache_status_t;

/**
 * @brief caches page from disk
 */
typedef struct lh_page_cache {
	cache_status_t	status;			/**< status of page cache */
	int				bucket_idx;		/**< idx of page that is in cache */
	l_hash_bucket_t *cached_bucket;
	/**< the actual data */
} lh_page_cache_t;

/**
@brief		Struct used to maintain an instance of an in memory hashmap.
*/
struct linear_hashmap {
	dictionary_parent_t super;
	int					initial_map_size;
	/**< The size of the map in item capacity */
	write_concern_t		write_concern;

	/**< The current @p write_concern level
			 of the hashmap*/
	err_t (*compute_hash)(
		linear_hashmap_t *,
		ion_key_t,
		int,
		int,
		hash_set_t *
	);

	/**< The hashing function to be used for
			 the instance.
			 In fact it will return both possible hash.*/
	char			*entry;			/**< Pointer to the entries in the hashmap*/
	FILE			*file;			/**< file pointer */
	int				file_level;		/**< the current file level for hash */
	int				bucket_pointer;
	/**< pointer for current bucket being spilt */
	int				id;				/**< id for files in system */
									/** @todo this could be moved to parent */
	int				record_size;	/**< the size of the record in the pp */
	int				number_of_records;
	/**< the number of records in the linear_hash */
	lh_page_cache_t cache[CACHE_SIZE];
	/**< holds pp for cacheing */
	char			use_split;		/**< controls splitying behaviour */
};

/**
 * @brief   Keeps track of the current ll file
 */
struct file_ll {
	FILE	*ll_file;				/**< The pointer for the current file open */
	int		bucket_id;				/**< The bucket ID for the file */
};

/**
@brief		This function initializes a linearu in memory hash map.

@param		hashmap
				Pointer to the hashmap instance to initialize.
@param		key_type
				The type of key that is being stored in the collection.
@param	  key_size
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
	linear_hashmap_t *hashmap,
	err_t (*compute_hash)(linear_hashmap_t *, ion_key_t, int, int, hash_set_t *),
	key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	int size,
	int id
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
	linear_hashmap_t *hash_map
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
	linear_hashmap_t *hash_map
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
err_t
lh_insert(
	linear_hashmap_t	*hash_map,
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
lh_update(
	linear_hashmap_t	*hash_map,
	ion_key_t			key,
	ion_value_t			value
);

/**
 * @brief	   Checks to see if the item in the cache can be updated and updates it
 *			  if a match.
 * @param	   hash_map
 * @param key
 * @param item
 * @param value
 * @return
 */
action_status_t
lh_update_item_action(
	linear_hashmap_t	*hash_map,
	ion_key_t			key,
	l_hash_bucket_t		*item,
	ion_value_t			value
);

/**
@brief	  Locates item in map.

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
		linear_hashmap_t	*hash_map,
		ion_key_t			key,
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
	linear_hashmap_t	*hash_map,
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
				The value associated in the map.   Returns the first valid value on a direct match.
*/
err_t
lh_query(
	linear_hashmap_t	*hash_map,
	ion_key_t			key,
	ion_value_t			value
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
	linear_hashmap_t	*hash_map,
	dict_cursor_t		*cursor
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
	linear_hashmap_t	*hash_map,
	int					size,
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
	linear_hashmap_t	*hashmap,
	ion_key_t			key,
	int					size_of_key,
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
	linear_hashmap_t *hash_map
);

/*void
static_hash_init(dictonary_handler_t * client);*/

err_t
lh_get_next(
	linear_hashmap_t	*hash_map,
	ll_file_t			*linked_list_file,
	ion_key_t			key,
	ion_value_t			value
);

int
lh_compute_bucket_number(
	linear_hashmap_t	*hash_map,
	hash_set_t			*hash_set
);

action_status_t
lh_insert_item_action(
	linear_hashmap_t	*hash_map,
	ion_key_t			key,
	l_hash_bucket_t		*item,
	ion_value_t			value
);

action_status_t
lh_delete_item_action(
	linear_hashmap_t	*hash_map,
	ion_key_t			key,
	l_hash_bucket_t		*item,
	ion_value_t			value
);

action_status_t
lh_query_item_action(
	linear_hashmap_t	*hash_map,
	ion_key_t			key,
	l_hash_bucket_t		*item,
	ion_value_t			value
);

action_status_t
lh_split_item_action(
	linear_hashmap_t	*hash_map,
	ion_key_t			key,
	l_hash_bucket_t		*item,
	ion_value_t			value
);

/**
 * Brings a primary page into the cache
 * @param hash_map
 * @param bucket_number
 * @return
 *
 * @TODO Add age detection to cache buckets to determine if a bucket has been written or not.
 * If the primary page has not been written since it have been last flushed, then just drop it
 * and go onto the next page
 */
err_t
lh_cache_pp(
	linear_hashmap_t	*hash_map,
	int					cache_number,
	int					bucket_number,
	lh_page_cache_t		**cache
);

/**
 * @brief   Accesses a memory location in the cache.
 *		  This does not actually copy the value out, but
 *		  points to a memory location is the cache.
 *		  The size of the data is already know.
 *
 * @param   hash_map
 *			  The hashmap the cache is associated with
 * @param   cache_number
 *			  The cache number to operate on
 * @param   idx
 *			  The position of the data in the cache, specifically the record number in the pp
 * @param   item
 *			  The pntr that will used to reference the item in the cache
 * @return
 */
err_t
lh_read_cache(
	linear_hashmap_t	*hash_map,
	lh_page_cache_t		*cache,
	int					idx,
	void				**item
);

/**
 * @brief	Changes the status of a record in the cache
 * @param cache
 * @param item
 * @param status
 * @return
 */
err_t
lh_write_record_status_in_cache(
	linear_hashmap_t	*hash_map,
	l_hash_bucket_t		*item,
	char				status
);

/**
 * @brief	Writes data to cache and updates cache status bits
 *		  which will be used by system during cache flushing
 *		  to determine is cache actually needs to be flushed.
 *
 * @param   hash_map
 *			  The hashmap the cache is associated with
 * @param   cache_number
 *			  The cache number to operate on
 * @param   to
 *			  The destination of where the data will be written
 * @param	from
 *			  The location of the the data to be read
 * @param   length
 *			  The length in bytes of the data
 * @return
 */
err_t
lh_write_cache_raw(
	linear_hashmap_t	*hash_map,
	/**int					cache_number,*/
	lh_page_cache_t		*cache,
	void				*to,
	l_hash_bucket_t		*from,
	int					length
);

err_t
lh_write_cache_record(
	linear_hashmap_t	*hash_map,
	l_hash_bucket_t		*item,
	ion_key_t			key,
	ion_value_t			value
);

/**
 * @brief flushes a pp back to disk and clears up cach
 * @param hash_map
 * @param cache
 * @param action
 * @return
 */
err_t
lh_flush_cache(
	linear_hashmap_t	*hash_map,
	lh_page_cache_t		*cache,
	int					action
);

ion_status_t
lh_action_primary_page(
	linear_hashmap_t *hash_map,
	lh_page_cache_t *cache,
	int bucket,
	ion_key_t key,
	action_status_t (*action)(linear_hashmap_t *, ion_key_t, l_hash_bucket_t *, ion_value_t),
	ion_value_t value
);

/** @FIXME - Make sure that value is malloc'd before call? */
/**
 * @brief - this searches for the value but does not get it but only records
 * the location.
 * @FIXME - not really happy with this.
 * @param hash_map
 * @param cache_number
 * @param cursor
 * @return
 */
err_t
lh_search_primary_page(
	linear_hashmap_t	*hash_map,
	lh_page_cache_t		*cache,
	lhdict_cursor_t		*cursor		/*predicate is in here */
);

/**
 * @brief	Determines the current load factor in the hashmap as
 *
 *			  load_factor = number_of_records/(page_size*number_of_primary_pages).
 *
 *		  The load factor does not take into consideration the number of
 *		  available pages available in overflow files.
 *
 * @param   hash_map
 *			  The hashmap to compute the load factor for.
 * @return
 */
int
lh_compute_load_factor(
	linear_hashmap_t *hash_map
);

#if defined(__cplusplus)
}
#endif

#endif /* LINEAR_HASH_H_ */
