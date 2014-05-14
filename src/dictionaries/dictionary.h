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

/**
@brief		Insert a value into a dictionary.
@param		handler
			A pointer to the handler for the dictionary to insert
			into.
@param		key
			The key that identifies @p value.
@param		value
			The value to store under @p key.
@returns	A status describing the result of the call.
*/
status_t
dictionary_insert(
	dictionary_t	*handler,
	key_t		key,
	value_t		value
);

/**
@brief		Retrieve the value data given an iterator
		state.
@param
		iterator
			A pointer to the dictionary iterator
			whose cursor is pointing at the item to
			retrieve.
@param		value
			A pointer the memory segment to write
			the retrieved value to.
@returns	A status describing the result of the call.
*/
status_t
dictionary_get(
	dict_iterator_t	*iterator,
	value_t		*value
);

/**
@brief		Delete values from a dictionary.
@param		iterator
			A pointer to a dictionary iterator whose
			cursor will be on the first item to delete
			after the first advancement.
@param		howmany
			The number items to delete from the dictionary.
@param		
*/
status_t
dictionary_delete(
	dict_iterator_t	*iterator,
	unsigned int	howmany
);

/**
@brief		Update values in a dictionary.
@param		iterator
			A pointer to a dictionary iterator whose
			cursor will be on the first item to update
			after the first advancement.
@param		howmany
			The number items to update from the dictionary.
@param		
*/
status_t
dictionary_update(
	dict_iterator_t	*iterator,
	unsigned int	howmany
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
dict_iterator_t
dictionary_find(
	dictionary_t	*handler,
	key_t		key
);

/**
@brief		Advance a dictionary iterator by one value.
@param		iterator
			A pointer to the dictionary iterator to move to next
			of.
@returns	@c TRUE if another value is available, @c FALSE otherwise.
		Ensure to check the status member of the iterator when done!
*/
bool_t
dictionary_next(
	dict_iterator_t	*iterator
);

#ifdef __cplusplus
}
#endif

#endif
