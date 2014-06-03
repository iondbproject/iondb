/*
 * oadictionaryhandler.c
 *
 *  Created on: May 26, 2014
 *      Author: workstation
 */

#include "oadictionaryhandler.h"
#include "./../../system.h"

//for each handler, need name and
typedef struct oa_dictionary
{
	//what needs to go in here?
	char * dictionary_name;
	hashmap_t * hashmap;			//this is the map that the operations will operate upon

}oa_dictionary_t;

//registers functions for handlers -> only needs to be done once?
err_t
oadict_init(dictionary_handler_t * handler)
{
	handler->insert = oadict_insert;
	handler->create_dictionary = oadict_create_dictionary;
	handler->get = oadict_query;
	handler->update = oadict_update;
	//handler->find =
	//handler->next =
	handler->delete = oadict_delete;
	handler->delete_dictionary = oadict_delete_dictionary;

	//TODO return type
}

err_t
oadict_insert(
	dictionary_t * dictionary,
	key_t *key,
	value_t	*value)
{
	return oah_insert((hashmap_t *)dictionary->instance,(char *)key, (char *)value);
}

err_t
oadict_query(
	dictionary_t *dictionary,
	key_t *key,
	value_t	**value)
{
	return oah_query((hashmap_t *)dictionary->instance,(char *)key, (char **)value);
}

// this will create a dictionary
err_t
oadict_create_dictionary(
		int key_size,
		int value_size,
		int dictionary_size,
		dictionary_handler_t * handler,		//handler for a specific type of dictionary
		dictionary_t * dictionary)
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
		dictionary_t * dictionary,
		key_t * key)
{
	return oah_delete((hashmap_t *)dictionary->instance,(char *)key);
}

err_t
oadict_delete_dictionary(
		dictionary_t * dictionary)
{
	err_t result = oah_destroy((hashmap_t *)dictionary->instance);
	free(dictionary->instance);
	return result;
}


err_t
oadict_update(dictionary_t * dictionary, key_t * key , value_t * value)
{
	return oah_update((hashmap_t *)dictionary->instance, (char *)key, (char *)value);
}
