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

#include "flat_file_types.h"
#include "flat_file.h"

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
ffdict_get(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief		Creates an instance of a flat file backed dictionary.
@param[in]	id
				The ID to assign to the dictionary. This is either user defined or is given
				by the master table.
@param[in]	key_type
				The category of key used by the dictionary. See @ref key_type for more information.
@param[in]	key_size
				The size of the keys used for this dictionary, specified in bytes. It is strongly
				recommended to use a @p sizeof() directive to specify this size to avoid painful problems.
@param[in]	value_size
				Same as above, for values.
@param[in]	dictionary_size
				Designates how many records we buffer in memory. Higher means better overall performance at
				the cost of increased memory usage.
@param[in]	compare
				The function pointer that designates how to compare two keys. This is given by the upper
				dictionary layers.
@param[in]	handler
				The allocated, initialized handler that will be bound to the dictionary instance.
@param[in]	dictionary
				The allocated dictionary instance that we are going to initialize.
@return		The resulting status of the operation.
*/
ion_err_t
ffdict_create_dictionary(
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size,
	ion_dictionary_compare_t	compare,
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary
);

/**
@brief		Removes all instances of any record with key equal to @p key.
@param[in]	dictionary
				Which dictionary to delete from.
@param[in]	key
				Designated key to look for and remove.
@return		The resulting status of the operation.
*/
ion_status_t
ffdict_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
);

/**
@brief		Cleans up all files created by the dictionary, and frees any allocated memory.
@param[in]	dictionary
				Which dictionary instance to delete.
@return		The resulting status of the operation.
*/
ion_err_t
ffdict_delete_dictionary(
	ion_dictionary_t *dictionary
);

/**
@brief		Updates all records stored at @p key to have value equal to @p value.
@details	If no records are stored at @p key, then an "upsert" (insert instead of update)
			is performed.
@param[in]	dictionary
				Which dictionary to update.
@param[in]	key
				Target key to update.
@param[in]	value
				New value to update to.
@return		The resulting status of the operation.
*/
ion_status_t
ffdict_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief			Initializes a cursor query and returns an allocated cursor object.
@details		Given a @p predicate that was previously initialized by @ref dictionary_build_predicate,
				Build a cursor object that acts as the iterator for the desired query through the dictionary.
@param[in]		dictionary
					Which dictionary to query on.
@param[in]		predicate
					An allocated, initialized predicate object that defines the parameters of the query.
@param[out]		cursor
					A cursor pointer should be initialized to @p NULL, and then given to this function.
					This function shall allocate appropriate memory and redirect @p cursor to point to
					the allocated memory. **NOTE:** Anything originally pointed to by this cursor will
					effectively be lost, if there is no other reference to said thing.
@return			The resulting status of the operation.
*/
ion_err_t
ffdict_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
);

/**
@brief			Fetches the next record to be returned from a cursor that has already been initialized.
@details		The returned record is written back to @p record, and then the cursor is advanced to the next
				record. The returned status code signifies whether or not there are more results to traverse.
				This function should not be called directly, but instead will be bound to the cursor like a method.
@param[in]		cursor
					Which cursor to fetch results from.
@param[out]		record
					An initialized record struct with the @p key and @p value appropriately allocated to fit
					the returned key and value. This function will write back data to the struct.
@return			The resulting status of the operation.
*/
ion_cursor_status_t
ffdict_next(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
);

/**
@brief		Destroys and frees the given cursor.
@details	This function should not be called directly, but instead accessed through the interface
			the cursor object.
@param[in]	cursor
				Which cursor to destroy.
*/
void
ffdict_destroy_cursor(
	ion_dict_cursor_t **cursor
);

#if defined(__cplusplus)
}
#endif

#endif
