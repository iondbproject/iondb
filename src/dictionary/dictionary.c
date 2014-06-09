/******************************************************************************/
/**
@file
@author		Graeme Douglas, Scott Fazackerley
@see		For more information, refer to @ref dictionary.c.
*/
/******************************************************************************/

#include "dictionary.h"


// creates a named instance of a database
status_t
dictionary_create(
		dictionary_handler_t 	*handler,
		dictionary_t 			*dictionary,
		int 					key_size,
		int 					value_size,
		int 					dictionary_size
)
{
	//using the handler, create the dictionary
	return handler->create_dictionary(key_size, value_size, dictionary_size, handler, dictionary);
}

//inserts a record into the dictionary
//each dictionary will have a specific handler?
status_t
dictionary_insert(
	/*dictionary_handler_t	*handler,*/
	dictionary_t 				*dictionary,
	ion_key_t 					*key,
	ion_value_t 				*value)
{
	return dictionary->handler->insert(dictionary, key, value);
}

status_t
dictionary_get(
	dictionary_t 				*dictionary,
	ion_key_t 					*key,
	ion_value_t 				**value
)
{
	return dictionary->handler->get(dictionary, key, value);
}

status_t
dictionary_update(
		dictionary_t 			*dictionary,
		ion_key_t 				*key,
		ion_value_t 			*value)
{
	return dictionary->handler->update(dictionary, key, value);
}
