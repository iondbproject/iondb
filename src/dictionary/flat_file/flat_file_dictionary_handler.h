/******************************************************************************/
/**
@file		flat_file_dictionary_handler.h
@author		Eric Huang
@brief		Function declarations at the dictionary interface level for the
			flat file store.
@details	These functions are not intended to be used directly. The entry
			point here is the initialization function, which is used to bind
			these functions to a dictionary.
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
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
				The category of key used by the dictionary. See @ref ION_KEY_TYPE for more information.
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

#if defined(__cplusplus)
}
#endif

#endif
