/******************************************************************************/
/**
@file		dictionary.h
@author		Graeme Douglas, Scott Fazackerley
@brief		Interface defining how general dictionaries behave.
*/
/******************************************************************************/

#ifndef DICTIONARY_H
#define DICTIONARY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "./../kv_system.h"
#include "dicttypes.h"
#include <stdarg.h>

/**
@brief		Creates as instance of a specific type of dictionary.

@param
@return		A status describing the result of dictionary creation.
*/
status_t
dictionary_create(
	dictionary_handler_t	*handler,
	dictionary_t			*dictionary,
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
@brief		Retrieve the value data given an iterator
			state.

@param		cursor
				A pointer to the dictionary iterator
				whose cursor is pointing at the item to
				retrieve.
@param		value
				A pointer the memory segment to write
				the retrieved value to.
@returns	A status describing the result of the call.
*/
//todo  this will generate cursor -> need to decide how to deal with concurrency
status_t
dictionary_find(
	dictionary_t		*dictionary,
	predicate_t 		*predicate,
	dict_cursor_t 		*cursor
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
@brief		Retrieve a value given a key.

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
@brief		Get an iterator for all values with key @p key.
@param		handler
				A pointer to the handler for the dictionary to insert
				into.
@param		key
				The key search the dictionary for.
@returns	A dictionary iterator for all values with given key.
@todo		Must call next to setup first value?
*/
/*dict_iterator_t
dictionary_find(
	dictionary_t	*handler,
	key_t		key
);*/

/**
@brief		Advance a dictionary iterator by one value.
@param		iterator
			A pointer to the dictionary iterator to move to next
			of.
@returns	@c TRUE if another value is available, @c FALSE otherwise.
@todo		Ensure to check the status member of the iterator when done!
*/
/*bool_t
dictionary_next(
	dict_iterator_t	*iterator
);*/

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

 @param first_key
 @param second_key
 @param key_size
 @return
 */
char
dictionary_compare_char_array(
	ion_key_t 		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
);


/**
@brief		Allocates memory and creates predicate based on caller input.

@details	Builds predicate based on type requested by caller.  This
			function is variadic as it will take a different number of
			parameters depending on the type of predicate required.


@param 		dictionary
				The dictionary the predicate is being build for.
@param 		predicate
				The pointer to a predicate reference created by caller.
@param 		type
				The type of predicate being requested.
@param 		key
				The first key used to be used in the predicate.  If this
				is a single key comparison, then this is the only key
				required.  If this is a range query, the first key will
				be the lower bound as @p key >= value.  The first optional
				parameter will be used as the upper bound value.
@return		The status of the predicate.
 */
err_t
dictionary_build_predicate(
	dictionary_t			*dictionary,
	predicate_t				**predicate,
	predicate_operator_t	type,
	ion_key_t				key,
	...
);

/**
@brief			Destroys a predicate statement for a range query and cleans up
				memory

@details		Destroys a predicate statement for range query and cleans up
				memory internally allocated for specific instance.

@param 			predicate
					Pointer pointer to predicate to destroy.
 */
void
dictonary_destroy_predicate_range(
	predicate_t		**predicate
);

/**
@brief			Destroys a predicate statement for single operator and cleans up
				memory

@details		Destroys a predicate statement for single operator and cleans up
				memory internally allocated for specific instance.

@param 			predicate
					Pointer pointer to predicate to destroy.
*/
void
dictonary_destroy_predicate_statement(
	predicate_t		**predicate
);

#ifdef __cplusplus
}
#endif

#endif
