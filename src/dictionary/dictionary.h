/******************************************************************************/
/**
@file		dictionary.h
@author		Graeme Douglas
@brief		Interface defining how general dictionaries behave.
*/
/******************************************************************************/

#ifndef DICTIONARY_H
#define DICTIONARY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "./../system.h"
#include "dicttypes.h"

/**
 * Creates as instance of a specific type of dictionary
 * @return
 */
status_t
dictionary_create(
		dictionary_handler_t * handler,
		dictionary_t * dictionary,
		int key_size,
		int value_size,
		int dictionary_size
);

/**
@brief		Insert a value into a dictionary.
@param		collection
			The dictionary that the value is to be inserted to
@param		key
			The key that identifies @p value.
@param		value
			The value to store under @p key.
@returns	A status describing the result of the call.
*/
status_t
dictionary_insert(
	dictionary_t * dictionary,
	key_t * key,
	value_t	* value
);

/**
@brief		Retrieve the value data given an iterator
			state.

@param cursor_T
			A pointer to the dictionary iterator
			whose cursor is pointing at the item to
			retrieve.
@param	value
			A pointer the memory segment to write
			the retrieved value to.
@returns A status describing the result of the call.
*/
//todo  this will generate cursor -> need to decide how to deal with concurrency
status_t
dictionary_find(
	dictionary_t * dictionary,
	predicate_t * predicate,
	dict_cursor_t * cursor
);

/**
 *
 * @param collection
 * @param key
 * @param value
 * @return
 */
status_t
dictionary_get(
	dictionary_t * dictionary,
	key_t * key,
	value_t ** value
);

//TODO The behavior of this needs to be clarified
status_t
dictionary_delete(
	key_t * key
);

status_t
dictionary_update(
		dictionary_t *,
		key_t *,
		value_t *);


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
		Ensure to check the status member of the iterator when done!
*/
/*bool_t
dictionary_next(
	dict_iterator_t	*iterator
);*/

#ifdef __cplusplus
}
#endif

#endif
