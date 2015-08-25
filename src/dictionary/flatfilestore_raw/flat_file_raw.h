#ifndef FLATFILE_H_
#define FLATFILE_H_

#ifdef ARDUINO
#include "serial_c_iface.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "dicttypes.h"
#include "dictionary.h"
#include "kv_system.h"
#include "kv_io.h"

#include "file_handler_raw.h"

#define BLOCK_SIZE 512

#if !defined(USE_DATAFLASH_RAW) || !defined(USE_IONFS)
#define USE_SD
#endif /* !defined(USE_DATAFLASH_RAW) || !defined(USE_IONFS) */

#if defined(USE_DATAFLASH_RAW)
#include "dataflash_raw.h"
#elif defined(USE_DATAFLASH_FTL)
#include "ftl.h"
#elif defined(USE_IONFS)
#include "ionfs.h"
#elif defined(USE_SD_RAW)
#if defined(ARDUINO)
#include "sd_raw.h"
#else
#include "sd_raw_file.h"
#endif

#if defined(USE_DATAFLASH_RAW)
#define device_write(page, data, length, offset) (df_write(page, data, length, offset))
#define device_read(page, buffer, length, offset) (df_read(page, buffer, length, offset))
#define device_flush() (df_flush())
#elif defined(USE_SD_RAW)
#define device_write(page, data, length, offset) (sd_raw_write(page, data, length, offset))
#define device_read(page, buffer, length, offset) (sd_raw_read(page, buffer, length, offset))
#define device_flush() (sd_raw_flush())
#endif

/**
@brief		Struct used to maintain an instance of a flat file store.
*/
typedef struct ff_file
{
	dictionary_parent_t		super;
	write_concern_t 		write_concern;	/**< The current @p write_concern level
	 	 	 	 	 	 	 	 	 	 	 of the file*/
} ff_file_t;

/**
@brief		This function initializes a flat file.

@details	This initialises the file and assumes that the file
			does not exist.  If the file exists, it will not init.

@param		file
				Pointer to the file instance to initialize.
@param		compare
				Function pointer to the correct comparison function for the
				collection.
@param		key_type
				The type of key that is being stored in the collection.
@param 		key_size
				The size of the key in bytes.
@param		value_size
				The size of the value in bytes.
@return		The status describing the result of the initialization.
 */
err_t
ff_initialize(
	ff_file_t			*file,
	key_type_t			key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size
);

/**
@brief		Destroys the file

@details	Destroys the map file and frees the underlying memory.

@param		file
				The file to be destroyed.
@return		The status describing the result of the destruction
*/
err_t
ff_destroy(
	ff_file_t 	*file
);

/*
@brief		Insert record into file

@todo update comment
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
ff_insert(
	ff_file_t 		*file,
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
ff_update(
	ff_file_t 		*file,
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
ff_find_item_loc(
	ff_file_t 		*file,
	ion_key_t	 	key,
	uint32_t		*block_address,
	uint16_t		*block_current_record
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
return_status_t
ff_delete(
	ff_file_t 		*file,
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
ff_query(
	ff_file_t 		*file,
	ion_key_t 		key,
	ion_value_t 	value
);

#ifdef __cplusplus
}
#endif

#endif /* FLATFILE_H_ */
