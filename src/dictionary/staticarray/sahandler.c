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
	static_array_t *sa = (static_array_t *)dictionary->instance;
	return sa_get(sa,key,value);
}

status_t
sadict_insert(
	dictionary_t 	*dictionary,
	ion_key_t 			key,
	ion_key_t				value
)
{
	static_array_t *sa = (static_array_t *)dictionary->instance;
	return sa_insert(sa,key, value);
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
	static_array_t *st;

	st = malloc(sizeof(static_array_t));
	dictionary->instance = st;


	//this line made by raffi, check with someone
	sa_dictionary_create(st,key_size,value_size,dictionary_size);

	dictionary->handler = handler;
	return 0;
}

status_t
sadict_delete(
		dictionary_t 	*dictionary,
		ion_key_t 		key
)
{
	static_array_t *sa = (static_array_t *)dictionary->instance;
	return sa_delete(sa,key);
}

status_t
sadict_destroy(
		dictionary_t 	*dictionary
)
{
	status_t result = sa_destroy((static_array_t *)dictionary->instance);

	free(dictionary->instance);
	dictionary->instance = NULL;

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
