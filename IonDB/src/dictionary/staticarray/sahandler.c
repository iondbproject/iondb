/******************************************************************************/
/**
@file 		sahandler.c
@author		Raffi Kudlac
@brief		The handler for the static array
 */
/******************************************************************************/

#include "sahandler.h"
#define NULL ((void*) 0)

status_t
sa_init(
	dictionary_handler_t 	*handler
)
{
	handler->insert 			= sadict_insert;
	handler->create_dictionary 	= sadict_create;
	handler->get 				= sadict_query;
	handler->update 			= sadict_update;
	handler->find 				= sa_find;
	handler->delete 			= sadict_delete;
	handler->delete_dictionary 	= sadict_destroy;

	/**@TODO return type*/
	return status_ok;
}

status_t
sadict_query(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t		*value
)
{
	return sa_get(dictionary,key,value);
}

status_t
sadict_insert(
	dictionary_t 	*dictionary,
	ion_key_t 			key,
	ion_key_t				value
)
{
	return sa_insert(dictionary,key, value);
}

status_t
sadict_create(
		int 					key_size,
		int 					value_size,
		int 					dictionary_size,
		dictionary_handler_t 	*handler,
		dictionary_t 			*dictionary
)
{
	//this line made by raffi, check with someone
	sa_create(handler,dictionary,key_size,value_size,dictionary_size);
	return 0;
}

status_t
sadict_delete(
		dictionary_t 	*dictionary,
		ion_key_t 		key
)
{
	return sa_delete(dictionary,key);
}

status_t
sadict_destroy(
		dictionary_t 	*dictionary
)
{
	status_t result = sa_destroy(dictionary);
	return result;
}

status_t
sadict_update(
		dictionary_t 	*dictionary,
		ion_key_t 		key,
		ion_value_t 	value
)
{
	return sa_update(dictionary, key, value);
}

status_t
sadict_find(
		dictionary_t			*dictionary,
		predicate_t				*pred,
		dict_cursor_t			*cursor
)
{
	return NULL;
}
