/******************************************************************************/
/**
@file
@author		Graeme Douglas, Scott Fazackerley
@brief		Interface defining how general dictionaries behave.
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

#if !defined(DICTIONARY_H_)
#define DICTIONARY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdarg.h>
#include "../key_value/kv_system.h"
#include "dictionary_types.h"

/**
@brief			Given the ID, implementation specific extension, and a buffer to write to,
				writes back the formatted filename for any implementation instance.
@param[in]		id
					Given ID to use to generate a unique filename.
@param[in]		ext
					Given implementation specific filename extension to be used.
@param[out]		filename
					Char buffer to write-back into. This must be allocated memory.
@return			How many characters would have been written. It is a good idea to check that this does not exceed
				@ref ION_MAX_FILENAME_LENGTH.
*/
int
dictionary_get_filename(
	ion_dictionary_id_t id,
	char				*ext,
	char				*filename
);

/**
@brief		Creates as instance of a specific type of dictionary.
@details	This function is not to be used if you are using the master table.
@param		handler
				A pointer to a handler object containing pointers to
				all the functions necessary for this dictionary instance.
@param		dictionary
				A pointer to a dictionary object that will be used to
				access all dictionary operations.
@param		id
				The identifier used to identify the dictionary.
@param		key_type
				The type of the key.
@param		key_size
				The size of the key type to store.
@param		value_size
				The size of the value to store.
@param		dictionary_size
				The implementation specific dictionary size. The interpretation
				of this value is dependent on the dictionary implementaion
				being used.
@return		A status describing the result of dictionary creation.
*/
ion_err_t
dictionary_create(
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary,
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size
);

/**
@brief		Insert a value into a dictionary.

@param		dictionary
				The dictionary that the value is to be inserted to
@param		key
				The key that identifies @p value.
@param		value
				The value to store under @p key.
@returns	A status describing the result of the insertion.
*/
ion_status_t
dictionary_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief		Retrieve a value given a key.

@param		dictionary
				A pointer to the dictionary to be intialized.
@param		key
				The key to retrieve the value for.
@param		value
				A pointer to the value byte array to copy data into.
@return		A status describing the result of the retrieval.
*/
ion_status_t
dictionary_get(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief		Delete a value given a key.
@param		dictionary
				A pointer to the dictionary to be intialized.
@param		key
				The key to retrieve the value for.
@return		A status describing the result of the deletion.
*/
ion_status_t
dictionary_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
);

/**
@brief		Update all records with a given key.

@param		dictionary
				A pointer to the dictionary instance to update.
@param		key
				The key to identify records for updating.
@param		value
				The value to update records with.
*/
ion_status_t
dictionary_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief	  Destroys dictionary

@param	  dictionary
				The dictionary instance to destroy.
@return		The status of the total destruction of the dictionary.
*/
ion_err_t
dictionary_delete_dictionary(
	ion_dictionary_t *dictionary
);

/**
@brief		Compares two unsigned integer numeric keys
@details	Compares two ion_key_t assuming that they are of arbitrary
			length and integer, unsigned and numeric (ie not a char[]). The
			following values will be returned:

				@p first_key > @p second_key return 1
				@p first_key == @p second_key return 0
				@p first_key < @p second_key return -1

			This works for all integer numeric types for unsigned values
			as long as both keys are of the same type. You'll notice a weird
			math expression being expressed when computing the return value.
			This value is written in this a weird way to give us the desired
			{-1, 0, 1} range of return values. Draw out a table and the reasoning
			will become immediately obvious.
@param	  first_key
				The pointer to the first key in the comparison.
@param	  second_key
				The pointer to the second key in the comaparison.
@param	  key_size
				The length of the key in bytes.
@return		The resulting comparison value.
*/
char
dictionary_compare_unsigned_value(
	ion_key_t		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
);

/**
@brief		Compares two signed integer numeric keys
@details	Compares two ion_key_t assuming that they are of arbitrary
			length and integer, signed and numeric (ie not a char[]).  The
			following values will be returned:

				@p first_key > @p second_key return 1
				@p first_key == @p second_key return 0
				@p first_key < @p second_key return -1

			This works for all integer numeric types for signed
			values as long as both keys are of the same type.
@param	  first_key
				The pointer to the first key in the comparison.
@param	  second_key
				The pointer to the second key in the comparison.
@param	  key_size
				The length of the key in bytes.
@return		The resulting comparison value.
*/
char
dictionary_compare_signed_value(
	ion_key_t		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
);

/**
@brief		Compare any two character (byte) arrays. These are not assumed
			to be null-terminated.
@param		first_key
				The first (left) key being compared.
@param		second_key
				The second (right) key being compared.
@param		key_size
				The size of the keys being compared.
@return		The resulting comparison value.
*/
char
dictionary_compare_char_array(
	ion_key_t		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
);

/**
@brief		Compare any two null-terminated strings.
@param		first_key
				The first (left) key being compared.
@param		second_key
				The second (right) key being compared.
@param		key_size
				The (maximum) size of the keys being compared.
@return		The resulting comparison value.
*/
char
dictionary_compare_null_terminated_string(
	ion_key_t		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
);

/**
@brief		Opens a dictionary, given the desired config.
@param		handler
				A pointer to the dictionary handler object to be used.
@param		dictionary
				A pointer to the dictionary object to be manipulated.
@param		config
				A pointer to the configuration object to be used to open
				the dictionary with.
@returns	An error describing the result of open operation.
*/
ion_err_t
dictionary_open(
	ion_dictionary_handler_t		*handler,
	ion_dictionary_t				*dictionary,
	ion_dictionary_config_info_t	*config
);

/**
@brief		Closes a dictionary.
@param		dictionary
				A pointer to the dictionary object to be closed.
@returns	An error describing the result of open operation.
*/
ion_err_t
dictionary_close(
	ion_dictionary_t *dictionary
);

/**
@brief		Builds a predicate based on the type given.
@details	The caller is responsible for allocating the memory needed
			for the predicate.
@param		predicate
				A pointer to the pre-allocated predicate to be initialized.
@param		type
				The type of predicate to build.
@param		...
				Extra variables necessary for initializing the predicate.
				This depends on the type of predicate being initialized.
				Equality:	   1st vparam is target key.
				Range:		  1st vparam is lower bound, 2nd vparam is upper
								bound.
				All_records:	No vparams used.
				Predicate:	  TODO to be implemented
@returns	An error describing the result of open operation.
*/
ion_err_t
dictionary_build_predicate(
	ion_predicate_t			*predicate,
	ion_predicate_type_t	type,
	...
);

/**
@brief		Destroys an equality predicate.
@details	This function should not be called directly. Instead, it is set
			while building the predicate.
@param		predicate
				A pointer to the pointer to the predicate object being
				destroyed.
*/
void
dictionary_destroy_predicate_equality(
	ion_predicate_t **predicate
);

/**
@brief		Destroys a range predicate.
@details	This function should not be called directly. Instead, it is set
			while building the predicate.
@param		predicate
				A pointer to the pointer to the predicate object being
				destroyed.
*/
void
dictionary_destroy_predicate_range(
	ion_predicate_t **predicate
);

/**
@brief		Destroys an all records predicate.
@details	This function should not be called directly. Instead, it is set
			while building the predicate.
@param		predicate
				A pointer to the pointer to the predicate object being
				destroyed.
*/
void
dictionary_destroy_predicate_all_records(
	ion_predicate_t **predicate
);

/**
@brief		Uses the given predicate and cursor to search the dictionary.
@details	This function will allocate and initialize the cursor.
			This means that it must freed once we are done. This function
			sets up a cursor for traversal.
@param		dictionary
				A pointer to the dictionary object to be created.
@param		predicate
				A pointer predicate object to be used for the search.
@param		cursor
				A pointer to the pointer for a cursor object. Note that
				if the cursor pointer object is pointing to a valid cursor
				already, we will leak memory.
@returns	An error code describing the result of the operation.
*/
ion_err_t
dictionary_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
);

/**
@brief		Tests the supplied @p key against the predicate registered in the
			@p cursor. If the supplied @p cursor if of the type equality, the key is tested for equality with that
			cursor's equality value and returns boolean_true if the test passes. If the supplied @p cursor is
			of the type predicate_range, the key is tested for whether it falls in the range
			of that cursor's registered upper_bound and lower_bound and returns boolean_true if it does.
			In the case that the supplied @p cursor is of the type predicate_all_records, boolean_true is returned. If
			any of the above tests fail, or if the type of the @p cursor is not mentioned above, boolean_false is
			returned
@param	  cursor
				The cursor and predicate being used to test @p key against.
@param	  key
				The key to test.
@return		The result is the key passes or fails the predicate test.
*/
ion_boolean_t
test_predicate(
	ion_dict_cursor_t	*cursor,
	ion_key_t			key
);

#if defined(__cplusplus)
}
#endif

#endif
