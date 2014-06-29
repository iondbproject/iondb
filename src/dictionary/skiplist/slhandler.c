/******************************************************************************/
/**
@file		slhandler.c
@author		Kris Wallperington
@brief		Handler liaison between dictionary API and skiplist implementation
*/
/******************************************************************************/

#include "slhandler.h"
#include "./../../kv_system.h"

#ifdef 	DEBUG
#define TO_IMPLEMENT(name) 	io_printf("%s\n", "This is '" name "', implement me")
#endif

err_t
sldict_init(
	dictionary_handler_t 	*handler
)
{
	handler->insert 			= sldict_insert;
	handler->get 				= sldict_query;
	handler->create_dictionary 	= sldict_create_dictionary;
	handler->delete 			= sldict_delete;
	handler->delete_dictionary 	= sldict_delete_dictionary;
	handler->update 			= sldict_update;

	return err_ok;
}

err_t
sldict_insert(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t 	value
)
{
	return sl_insert((skiplist_t *) dictionary->instance, key, value);
}

err_t
sldict_query(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t 	*value
)
{

#ifdef 	DEBUG
	TO_IMPLEMENT("sldict_query");
#endif

	return err_ok;
}

err_t
sldict_create_dictionary(
	int 					key_size,
	int 					value_size,
	int 					dictionary_size,
	dictionary_handler_t 	*handler,
	dictionary_t 			*dictionary
)
{
	int pnum, pden;

	/* TODO malloc error check */
	dictionary->instance = malloc(sizeof(skiplist_t));

	pnum 	= 1;
	pden 	= 4;

	/* TODO Should we handle the possible error code returned by this? If yes, what sorts of errors does it return? */
	err_t result = sl_initialize((skiplist_t *) dictionary->instance, key_size,
										value_size, dictionary_size, pnum, pden);

	dictionary->handler = handler;

	return result;
}

err_t
sldict_delete(
	dictionary_t 	*dictionary,
	ion_key_t 		key
)
{

#ifdef 	DEBUG
	TO_IMPLEMENT("sldict_delete");
#endif

	return err_ok;
}

err_t
sldict_delete_dictionary(
	dictionary_t 	*dictionary
)
{

	err_t result = sl_destroy((skiplist_t *) dictionary->instance);
	free(dictionary->instance);
	dictionary->instance = NULL;
	return result;
}

err_t
sldict_update(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t 	value
)
{

#ifdef 	DEBUG
	TO_IMPLEMENT("sldict_update");
#endif

	return err_ok;
}
