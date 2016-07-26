/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Function declarations at the dictionary interface level for the
			flat file store.
@details	These functions are not intended to be used directly. The entry
			point here is the initialization function, which is used to bind
			these functions to a dictionary.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
*/
/******************************************************************************/

#if !defined(FLAT_FILE_DICTIONARY_HANDLER_H_)
#define FLAT_FILE_DICTIONARY_HANDLER_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "flat_file.h"
#include "flat_file_types.h"
#include "../../file/SD_stdio_c_iface.h"

/**
@brief		Given the @p handler instance, bind the appropriate flat file functions.
@param[in]	handler
				The handler is assumed to be memory that is allocated and initialized
				by the user.
*/
void
ffdict_init(
	ion_dictionary_handler_t *handler
);

/**
@brief		Given a record ( @p key, @p value ), insert it into the dictionary.
@param[in]	dictionary
				The initialized dictionary instance we want to insert into.
@param[in]	key
				The key portion of the record to be inserted.
@param[in]	value
				The value portion of the record to be inserted.
@return		The resulting status of the operation.
*/
ion_status_t
ffdict_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief		Performs a "get" operation on the dictionary to retrieve a single record.
@details	Given a @p key, returns the associated value stored under
			that key. If there are duplicate records all with the same
			@p key, the exact one that is returned is undefined. If you need
			to get all records stored under a specific key, use a cursor query.
@param[in]	dictionary
				Which dictionary to perform the operation on.
@param[in]	key
				The desired search key.
@param[out]	value
				The output location in which to write the returned value. This space
				must be allocated by the user to at least @p value_size bytes, as
				originally defined on dictionary creation.
@return		The resulting status of the operation.
*/
ion_status_t
ffdict_query(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief		Creates an instance of a dictionary.

@details	Creates as instance of a dictionary given a @p key_size and
			@p value_size, in bytes as well as the @p dictionary size
			which is the number of buckets available in the hashmap.

@param		id
@param		key_type
@param	  key_size
				The size of the key in bytes.
@param	  value_size
				The size of the value in bytes.
@param	  dictionary_size
				The size of the hashmap in discrete units
@param		compare
				Function pointer for the comparison function for the dictionary.
@param	  handler
				 THe handler for the specific dictionary being created.
@param	  dictionary
				 The pointer declared by the caller that will reference
				 the instance of the dictionary created.
@return		The status of the creation of the dictionary.
*/
ion_err_t
ffdict_create_dictionary(
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	int							dictionary_size,	/* @todo this needs to be fixed or defined */
	ion_dictionary_compare_t	compare,
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary
);

/**
@brief		Deletes the @p key and assoicated value from the dictionary
			instance.

@param	  dictionary
				The instance of the dictionary to delete from.
@param	  key
				The key that is to be deleted.
@return		The status of the deletion
*/
ion_status_t
ffdict_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
);

/**
@brief	  Deletes an instance of the dictionary and associated data.

@param	  dictionary
				The instance of the dictionary to delete.
@return		The status of the dictionary deletion.
*/
ion_err_t
ffdict_delete_dictionary(
	ion_dictionary_t *dictionary
);

/**
@brief		Updates the value for a given key.

@details	Updates the value for a given @p key.  If the key does not currently
			exist in the hashmap, it will be created and the value sorted.

@param	  dictionary
				The instance of the dictionary to be updated.
@param	  key
				The key that is to be updated.
@param	  value
				The value that is to be updated.
@return		The status of the update.
*/
ion_status_t
ffdict_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief	  Finds multiple instances of a keys that satisfy the provided
			 predicate in the dictionary.

@details	Generates a cursor that allows the traversal of items where
			the items key satisfies the @p predicate (if the underlying
			implementation allows it).

@param	  dictionary
				The instance of the dictionary to search.
@param	  predicate
				The predicate to be used as the condition for matching.
@param	  cursor
				The pointer to a cursor which is caller declared but callee
				is responsible for populating.
@return		The status of the operation.
*/
ion_err_t
ffdict_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
);

/**
@brief		Next function to query and retrieve the next
			<K,V> that stratifies the predicate of the cursor.

@param	  cursor
				The cursor to iterate over the results.
@return		The status of the cursor.
*/
/*ion_err_t
oadict_next(
	ion_dict_cursor_t   *cursor,
	ion_value_t		*value
);*/

/**
@brief		Next function to query and retrieve the next
			<K,V> that stratifies the predicate of the cursor.

@param	  cursor
				The cursor to iterate over the results.
@param		record
@return		The status of the cursor.
*/
ion_cursor_status_t
ffdict_next(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
);

/**
@brief		Destroys the cursor.

@details	Destroys the cursor when the user is finished with it.  The
			destroy function will free up internally allocated memory as well
			as freeing up any reference to the cursor itself.  Cursor pointers
			will be set to NULL as per ION_DB specification for de-allocated
			pointers.

@param	  cursor
				** pointer to cursor.
*/
void
ffdict_destroy_cursor(
	ion_dict_cursor_t **cursor
);

/**
@brief		Tests the supplied @p key against the predicate registered in the
			cursor.

@param	  cursor
				The cursor and predicate being used to test @p key against.
@param	  key
				The key to test.
@return		The result is the key passes or fails the predicate test.
*/
ion_boolean_t
ffdict_test_predicate(
	ion_dict_cursor_t	*cursor,
	ion_key_t			key
);

/**

@brief			Starts scanning map looking for conditions that match
				predicate and returns result.

@details		Scans that map looking for the next value that satisfies the predicate.
				The next valid index is returned through the cursor

@param			cursor
					A pointer to the cursor that is operating on the map.

@return			The status of the scan.
*/
ion_err_t
ffdict_scan(
	ion_ffdict_cursor_t *cursor	/* don't need to pass in the cursor */
);

#if defined(__cplusplus)
}
#endif

#endif
