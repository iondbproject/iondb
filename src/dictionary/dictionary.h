/******************************************************************************/
/**
@file		dictionary.h
@author		Graeme Douglas, Scott Fazackerley
@brief		Interface defining how general dictionaries behave.
*/
/******************************************************************************/

#if !defined(DICTIONARY_H_)
#define DICTIONARY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdarg.h>
#include "./../kv_system.h"
#include "dictionary_types.h"

/**
@brief		Creates as instance of a specific type of dictionary.

@param
@return		A status describing the result of dictionary creation.
*/
status_t
dictionary_create(
	dictionary_handler_t	*handler,
	dictionary_t			*dictionary,
	ion_dictionary_id_t		id,
	key_type_t				key_type,
	int						key_size,
	int						value_size,
	int						dictionary_size
);

/**
@brief		Insert a value into a dictionary.

@param		collection
				The dictionary that the value is to be inserted to
@param		key
				The key that identifies @p value.
@param		value
				The value to store under @p key.
@returns	A status describing the result of the insertion.
*/
status_t
dictionary_insert(
	dictionary_t			*dictionary,
	ion_key_t				key,
	ion_value_t				value
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
@todo		@p key should probably not be pointer (as keys are defined to be pointers).
@todo		Same for @p value.
*/
status_t
dictionary_get(
	dictionary_t		*dictionary,
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
@todo		@p key should probably not be pointer (as keys are defined to be pointers).
*/
status_t
dictionary_delete(
	dictionary_t		*dictionary,
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
status_t
dictionary_update(
		dictionary_t	*dictionary,
		ion_key_t		key,
		ion_value_t		value
);

/**
@brief 		Destroys dictionay

@param 		dictionary
				The dictionary instance to destroy.
@return		The status of the total destruction of the dictionary.
 */
status_t
dictionary_delete_dictionary(
	dictionary_t		*dictionary
);


/**
@brief		Compares two unsigned integer numeric keys

@details	Compares two ion_key_t assuming that they are of arbitrary
			length and integer, unsigned and numeric (ie not a char[]).  The
			following values will be returned:

				@p first_key > @p second_key return 1
				@p first_key == @p second_key return 0
				@p first_key < @p second_key return -1

			This works for all integer numeric types for unsigned values
			as long as both keys are of the same type.

@param 		first_key
				The pointer to the first key in the comparison.

@param 		second_key
				The pointer to the second key in the comaparison.

@param 		key_size
				The length of the key in bytes.

@return		The resulting comparison value.
 */
char
dictionary_compare_unsigned_value(
	ion_key_t 		first_key,
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

@param 		first_key
				The pointer to the first key in the comparison.

@param 		second_key
				The pointer to the second key in the comparison.

@param 		key_size
				The length of the key in bytes.

@return		The resulting comparison value.
 */
char
dictionary_compare_signed_value(
	ion_key_t 		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
);

/**
@brief	Compare any two character (byte) arrays. These are not assumed
		to be null-terminated.
@param	first_key
				The first (left) key being compared.
@param	second_key
				The second (right) key being compared.
@param	key_size
				The size of the keys being compared.
@return	The resulting comparison value.
*/
char
dictionary_compare_char_array(
	ion_key_t 		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
);

/**
@brief	Compare any two null-terminated strings.
@param	first_key
				The first (left) key being compared.
@param	second_key
				The second (right) key being compared.
@param	key_size
				The (maximum) size of the keys being compared.
@return	The resulting comparison value.
*/
char
dictionary_compare_null_terminated_string(
	ion_key_t 		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
);

/**
@brief Opens a dictionary, given the desired config. 
*/
err_t
dictionary_open(
    dictionary_handler_t 			*handler,
    dictionary_t 					*dictionary,
    ion_dictionary_config_info_t 	*config
);

/**
@brief Closes a dictionary. 
*/
err_t
dictionary_close(
    dictionary_t 					*dictionary
);

/**
@brief 	Builds a predicate based on the type given.
		Equality: 		1st vparam is target key.
		Range: 			1st vparam is lower bound, 2nd vparam is upper bound.
		All_records: 	No vparams used.
		Predicate: 		TODO to be implemented
*/
err_t
dictionary_build_predicate(
	predicate_t				*predicate,
	predicate_type_t		type,
	...
);

/**
@brief Destroys an equality predicate.
*/
void
dictionary_destroy_predicate_equality(
		predicate_t **predicate
);

/**
@brief Destroys a range predicate.
*/
void
dictionary_destroy_predicate_range(
		predicate_t **predicate
);

/**
@brief Destroys an all records predicate.
*/
void
dictionary_destroy_predicate_all_records(
	predicate_t 	**predicate
);

/**
@brief Uses the given predicate and cursor to search on the dictionary.
*/
err_t
dictionary_find(
	dictionary_t 	*dictionary,
	predicate_t 	*predicate,
	dict_cursor_t 	**cursor
);

#if defined(__cplusplus)
}
#endif

#endif
