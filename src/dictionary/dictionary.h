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

/**
@brief		Creates as instance of a specific type of dictionary.

@param
@return		A status describing the result of dictionary creation.
*/
status_t
dictionary_create(
	dictionary_handler_t	*handler,
	dictionary_t			*dictionary,
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
	ion_key_t				*key,
	ion_value_t				*value
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
	ion_key_t			*key,
	ion_value_t			**value
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
	ion_key_t			*key
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
		ion_key_t		*key,
		ion_value_t		*value
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

#ifdef __cplusplus
}
#endif

#endif
