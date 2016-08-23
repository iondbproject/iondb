/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Implementation specific declarations for the flat file store.
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

#if !defined(FLAT_FILE_H)
#define FLAT_FILE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "flat_file_types.h"

/**
@brief		Initializes the flat file implementation and creates all necessary files.
@details	A check is done to see if this is actually an attempt to open a previously existing
			flat file instance. This (should) only happen when this is called from an open context
			instead of an initialize. The flat file supports a special mode called "sorted mode". This
			is an append only mode that assumes all keys come in monotonic non-decreasing order. In this
			mode, search operations are significantly faster, but the store does not support deletions while
			in sorted mode.
@param[in]	flat_file
				Given instance of a flat file struct to initialize. This must be allocated **heap** memory,
				as destruction will assume that it needs to be freed.
@param[in]	id
				The assigned ID of this dictionary instance.
@param[in]	key_type
				Key category to use for this instance.
@param[in]	key_size
				Key size, in bytes used for this instance.
@param[in]	value_size
				Value size, in bytes used for this instance.
@param[in]	dictionary_size
				Dictionary size is interpreted as how many records (key value pairs) are buffered. This should be given
				as somewhere between 1 (minimum) and the page size of the device you are working on.
@return		The status of initialization.
@see		ffdict_create_dictionary
*/
ion_err_t
flat_file_initialize(
	ion_flat_file_t			*flat_file,
	ion_dictionary_id_t		id,
	ion_key_type_t			key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size
);

/**
@brief		Destroys and cleans up any implementation specific memory or files.
@param[in]	flat_file
				Given flat file instance to destroy.
@return		The resulting status of destruction.
@see		ffdict_delete_dictionary
*/
ion_err_t
flat_file_destroy(
	ion_flat_file_t *flat_file
);

/**
@brief		Inserts the given record into the flat file store.
@param[in]	flat_file
				Which flat file to insert into.
@param[in]	key
				Key portion of the record to insert.
@param[in]	value
				Value portion of the record to insert.
@return		Resulting status of insertion.
@see		ffdict_insert
*/
ion_status_t
flat_file_insert(
	ion_flat_file_t *flat_file,
	ion_key_t		key,
	ion_value_t		value
);

/**
@brief		Fetches the record stored with the given @p key.
@param[in]	flat_file
				Which flat file to look in.
@param[in]	key
				Specified key to look for.
@param[out]	value
				Value portion of the record to insert.
@return		Resulting status of the operation.
@see		ffdict_get
@todo		Write tests for sorted mode get.
*/
ion_status_t
flat_file_get(
	ion_flat_file_t *flat_file,
	ion_key_t		key,
	ion_value_t		value
);

/**
@brief		Deletes all records stored with the given @p key.
@param[in]	flat_file
				Which flat file to delete in.
@param[in]	key
				Specified key to find and delete.
@return		Resulting status of the operation.
@see		ffdict_delete
*/
ion_status_t
flat_file_delete(
	ion_flat_file_t *flat_file,
	ion_key_t		key
);

/**
@brief		Updates all records stored with the given @p key to have @p value.
@param[in]	flat_file
				Which flat file to update in.
@param[in]	key
				Specified key to find and update.
@param[in]	value
				New value to replace old values with.
@return		Resulting status of the operation.
@see		ffdict_update
@todo		Write tests for sorted mode update.
*/
ion_status_t
flat_file_update(
	ion_flat_file_t *flat_file,
	ion_key_t		key,
	ion_value_t		value
);

/**
@brief		Closes and frees any memory associated with the flat file.
@param		flat_file
				Which flat file to close.
@return		Status of closure.
*/
ion_err_t
flat_file_close(
	ion_flat_file_t *flat_file
);

/**
@brief			Performs a linear scan of the flat file, going forwards
				if @p scan_forwards is true, writing the first location
				seen that satisfies the given @p predicate to @p location.
@details		If the scan falls through, then the location is written as
				the EOF position of the file. The variadic arguments accepted
				by this function are passed into the predicate's additional parameters.
				This can be used to provide additional context to the predicate for use
				in determining whether or not the row is a match.
@param[in]		flat_file
					Which flat file instance to scan.
@param[in]		start_location
					Where to begin the scan. This is given as a row index. If
					given as -1, then it is assumed to be either the start of
					file or end of file, depending on the state of @p scan_forwards.
@param[out]		location
					Allocated memory location to write back the found location into.
					Is not changed in the event of a failure or error condition. This
					location is given back as a row index.
@param[out]		row
					A row struct to write back the found row into. This is allocated
					by the user.
@param[in]		scan_forwards
					Scans front-to-back if @p true, else scans back-to-front.
@param[in]		predicate
					Given test function to check each row against. Once this function
					returns true, the scan is terminated and the found location and row
					are written back to their respective output parameters.
@return			Resulting status of scan.
@todo			Try changing the predicate to be an enum-and-switch to eliminate the function
				call. Benchmark the performance gain and decide which strategy to use.
@todo			Consider changing to @p SEEK_CUR whenever possible. Benchmark this and see
				if the performance gain (if any) is worth it.
*/
ion_err_t
flat_file_scan(
	ion_flat_file_t				*flat_file,
	ion_fpos_t					start_location,
	ion_fpos_t					*location,
	ion_flat_file_row_t			*row,
	ion_boolean_t				scan_forwards,
	ion_flat_file_predicate_t	predicate,
	...
);

/**
@brief		Predicate function to return any row that has an exact match to the given target key.
@details	We expect one @ref ion_key_t to be in @p args.
@see		ion_flat_file_predicate_t
*/
ion_boolean_t
flat_file_predicate_key_match(
	ion_flat_file_t		*flat_file,
	ion_flat_file_row_t *row,
	va_list				*args
);

/**
@brief		Predicate function to return any row that has a key such that
			`lower_bound <= key <= upper_bound` holds true.
@details	We expect two @ref ion_key_t parameters to be in @p args - the first
			to represent the lower bound of the key, and the second to represent the
			upper bound.
@see		ion_flat_file_predicate_t
*/
ion_boolean_t
flat_file_predicate_within_bounds(
	ion_flat_file_t		*flat_file,
	ion_flat_file_row_t *row,
	va_list				*args
);

/**
@brief		Predicate function to return any row that is **not** empty or deleted.
@see		ion_flat_file_predicate_t
*/
ion_boolean_t
flat_file_predicate_not_empty(
	ion_flat_file_t		*flat_file,
	ion_flat_file_row_t *row,
	va_list				*args
);

/**
@brief		Reads the row specified by the given location into the buffer.
@details	The returned row is given by attaching pointers correctly from
			the given row parameter. These pointers are associated with the
			internal read buffer of the flat file. You should assume that the
			row does not have a lifetime beyond the scope of where you call
			this function - any subsequent operation that mutates the read buffer
			will cause the row to become garbage. Copy the row data out if you want
			it to persist. If the requested row has already been loaded by a prior
			call to @ref flat_file_scan, then it will retrieve it as a cache hit
			directly from the buffer. Otherwise, it will do a seek and read to fetch
			the row.
@param[in]	flat_file
				Which flat file instance to read from.
@param[in]	location
				Which row index to read. This function will compute
				the file offset of the row index.
@param[in]	row
				Write back row to place read data from the desired @p location.
@return		Resulting status of the several file operations used to perform the read.
*/
ion_err_t
flat_file_read_row(
	ion_flat_file_t		*flat_file,
	ion_fpos_t			location,
	ion_flat_file_row_t *row
);

/**
@brief		Performs a binary search for the given @p target_key, returning to @p location
			the first-less-than-or-equal key within the flat file. This can only be used if
			@p sorted_mode is enabled within the flat file.
@details	In the case of duplicates, this function will scroll to the beginning of the block
			of duplicates, before writing back to @p location. As a result, it is guaranteed that
			the returned index points to the first key in a contiguous block of duplicate keys. If
			no key in the flat file satisfies the condition of being less-than-or-equal, then @p -1
			is written back to @p location. This function will only return records that are not deleted.
@param[in]		flat_file
				Which flat file instance to search within.
@param[in]		target_key
				Desired key to search for.
@param[out]		location
				Found location to write back into. Must be allocated by caller.
@return		Resulting status of the binary search operation.
*/
ion_err_t
flat_file_binary_search(
	ion_flat_file_t *flat_file,
	ion_key_t		target_key,
	ion_fpos_t		*location
);

#if defined(__cplusplus)
}
#endif

#endif
