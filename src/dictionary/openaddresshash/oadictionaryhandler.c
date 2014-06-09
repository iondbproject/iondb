/******************************************************************************/
/**
@file 		oadictionaryhandler.c
@author		Scott Ronald Fazackerley
@brief		The handler for a hash table using linear probing.
 */
/******************************************************************************/

#include "oadictionaryhandler.h"
#include "./../../kv_system.h"


err_t
oadict_init(
	dictionary_handler_t 	*handler
)
{
	handler->insert 			= oadict_insert;
	handler->create_dictionary 	= oadict_create_dictionary;
	handler->get 				= oadict_query;
	handler->update 			= oadict_update;
	//handler->find =
	//handler->next =
	handler->delete 			= oadict_delete;
	handler->delete_dictionary 	= oadict_delete_dictionary;

	/**@TODO return type*/
	return 0;
}

err_t
oadict_insert(
	dictionary_t 	*dictionary,
	ion_key_t 		*key,
	ion_value_t		*value
)
{
	return oah_insert((hashmap_t *)dictionary->instance,(char *)key, (char *)value);
}

err_t
oadict_query(
	dictionary_t 	*dictionary,
	ion_key_t 		*key,
	ion_value_t		**value
)
{
	return oah_query((hashmap_t *)dictionary->instance,(char *)key, (char **)value);
}


err_t
oadict_create_dictionary(
		int 					key_size,
		int 					value_size,
		int 					dictionary_size,
		dictionary_handler_t 	*handler,
		dictionary_t 			*dictionary
)
{
	//this is the instance of the hashmap
	dictionary->instance = (hashmap_t *)malloc(sizeof(hashmap_t));

	//this registers the dictionarys the dictionary
	oah_initialize((hashmap_t *)dictionary->instance, oah_compute_simple_hash, key_size, value_size, dictionary_size);    			// just pick an arbitary size for testing atm

	//register the correct handler
	dictionary->handler = handler;		//todo: need to check to make sure that the handler is registered

	return 0;
}

err_t
oadict_delete(
		dictionary_t 	*dictionary,
		ion_key_t 		*key
)
{
	return oah_delete((hashmap_t *)dictionary->instance,(char *)key);
}

err_t
oadict_delete_dictionary(
		dictionary_t 	*dictionary
)
{
	err_t result = oah_destroy((hashmap_t *)dictionary->instance);
	free(dictionary->instance);
	return result;
}


err_t
oadict_update(
		dictionary_t 	*dictionary,
		ion_key_t 		*key,
		ion_value_t 	*value
)
{
	return oah_update((hashmap_t *)dictionary->instance, (char *)key, (char *)value);
}
