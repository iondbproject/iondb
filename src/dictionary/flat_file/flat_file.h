/*
 * flat_file.h
 *
 *  Created on: Aug 14, 2014
 *	  Author: workstation
 */

#if !defined(FLAT_FILE_H_)
#define FLAT_FILE_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#include "../dictionary_types.h"
#include "./../dictionary.h"
#include "file_handler.h"

#include "../../key_value/kv_system.h"
#include "../../key_value/kv_io.h"

/* redefines file operations for arduino */
#include "./../../file/SD_stdio_c_iface.h"

typedef long int ion_fpos_t;/**< Flatfile file position type */

/**
@brief		Struct used to maintain an instance of a flat file store.
*/
typedef struct ff_file {
	ion_dictionary_parent_t super;
	FILE					*file_ptr;	/**< Pointer to file store */
	ion_fpos_t				start_of_data;	/**< indicates start of data block */
	ion_write_concern_t		write_concern;		/**< The current @p write_concern level
												of the file*/
} ion_ff_file_t;

/**
@brief		This function initializes a flat file.

@details	This initialises the file and assumes that the file
			does not exist.  If the file exists, it will not init.

@param		file
				Pointer to the file instance to initialize.
@param		key_type
				The type of key that is being stored in the dictionary instance.
@param		key_size
				The size of the key in bytes.
@param		value_size
				The size of the value in bytes.
@return		The status describing the result of the initialization.
 */
ion_err_t
ff_initialize(
	ion_ff_file_t		*file,
	ion_key_type_t		key_type,
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
ion_err_t
ff_destroy(
	ion_ff_file_t *file
);

/**
@brief		Insert record into file

@todo update comment - THIS COMMENT REFERS TO A HASH STRUCTURE WHICH IS COMPLETELY INCORRECT!
@details	Attempts to insert data of a given structure as dictated by record
			into the provided hashmap.  The record is used to determine the
			structure of the data <K,V> so that the key can be extracted.  The
			function will return the status of the insert.  If the record has
			been successfully inserted, the status will reflect success.  If
			the record can not be successfully inserted the error code will
			reflect failure.  Will only allow for insertion of unique records.

@param	  file
				The flatfile into which the data is going to be inserted.
@param		key
				The key that is being used to locate the position of the data.
@param		value
				The value that is being inserted.
@return	 The status of the insert.
*/
ion_status_t
ff_insert(
	ion_ff_file_t	*file,
	ion_key_t		key,
	ion_value_t		value
);

/**
@brief		Updates a value in the map.

FIXME THIS DOCUMENTATION IS ALL WRONG!

@details	Updates a value in the map.  If the value does not exist, it will
			insert the value.

@param		file
				The flatfile into which the data is going to be inserted.
@param		key
				The key that is being used to locate the position of the data.
@param		value
				The value that is being inserted.
@return		The status of the update
*/
ion_status_t
ff_update(
	ion_ff_file_t	*file,
	ion_key_t		key,
	ion_value_t		value
);

/**
@brief	  Locates item in map.

@details	Based on a key, function locates the record in the map.

@param		file
				The map into which the data is going to be inserted.
@param		key
				The key for the record that is being searched for.
@param		location
				Pointer to the location variable
@return		The status of the find
 */
ion_err_t
ff_find_item_loc(
	ion_ff_file_t	*file,
	ion_key_t		key,
	ion_fpos_t		*location
);

/**
@brief		Deletes item from map.

@details	Deletes item from map based on key.  If key does not exist
			error is returned

@param		file
				The flatfile into which the data is going to be inserted.
@param		key
				The key for the record that is being searched for.
*/
ion_status_t
ff_delete(
	ion_ff_file_t	*file,
	ion_key_t		key
);

/**
@brief		Locates the record if it exists.

@details	Locates the record based on key match is it exists and returns a
			pointer to the record that has been materialized in memory.
			This presents a significant issue as both the key and value could
			be modified, causing issues with map.

@param		file
				The flatfile into which the data is going to be inserted.
@param		key
				The key for the record that is being searched for.
@param		value
				The value associated in the map.
*/
ion_status_t
ff_query(
	ion_ff_file_t	*file,
	ion_key_t		key,
	ion_value_t		value
);

#if defined(__cplusplus)
}
#endif

#endif /* FLAT_FILE_H_ */
