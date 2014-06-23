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
	ion_key_t 		key,
	ion_value_t		value
)
{
	return oah_insert((hashmap_t *)dictionary->instance,(char *)key, (char *)value);
}

err_t
oadict_query(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t		*value
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
		ion_key_t 		key
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
	dictionary->instance = NULL;					// When releasing memory, set pointer to NULL
	return result;
}


err_t
oadict_update(
		dictionary_t 	*dictionary,
		ion_key_t 		key,
		ion_value_t 	value
)
{
	return oah_update((hashmap_t *)dictionary->instance, (char *)key, (char *)value);
}


err_t
oadict_find(
		dictionary_t 	*dictionary,
		predicate_t 	*predicate,
		dict_cursor_t 	*cursor
)
{
	//based on the type of predicate that is being used, need to create the correct cursor

	switch(predicate->type)
	{
		case predicate_equality:
		{
			//allocate memory for cursor
			if ((cursor = (dict_cursor_t *)malloc(sizeof(oadict_equality_cursor_t))) == NULL)
			{
				return err_out_of_memory;
			}
			//cast to specific instance type
			oadict_equality_cursor_t *eq_cursor = (oadict_equality_cursor_t *)(cursor);
			eq_cursor->super.dictionary = dictionary;
			eq_cursor->super.type = predicate->type;				/** types align */
			eq_cursor->super.status = cs_cursor_unitialized;
			//eq_cursor->cursor_info.current = NULL;
			//eq_cursor->cursor_info.first = NULL;
			//as this is an equality, need to malloc for key as well
			if ((eq_cursor->value = (ion_key_t)malloc(sizeof((((hashmap_t*)dictionary->instance)->record.key_size)))) == NULL)
			{
				return err_out_of_memory;
			}
			//copy across the key value as the predicate may be destroyed
			memcpy(eq_cursor->value, ((equality_statement_t *)predicate)->equality_value,(((hashmap_t*)dictionary->instance)->record.key_size));

			//find the location of the first element as this is a straight equality
			int location = cs_invalid_index;
			if (oah_find_item_loc((hashmap_t*)dictionary->instance, eq_cursor->value, &location) == err_item_not_found)
			{
				eq_cursor->super.status = cs_end_of_results;
				return err_ok;
			}
			else
			{
				// the cursor is ready to be consumed
				eq_cursor->cursor_info.first = location;
				eq_cursor->cursor_info.current = location;
				eq_cursor->cursor_info.status = cs_cursor_initialized;
				return err_ok;
			}
			break;
		}
		case predicate_range:
		{
			break;
		}
		case predicate_predicate:
		{
			break;
		}
		default:
			return err_invalid_predicate;		/** Invalid predicate supplied */
	}
	return err_ok;
}


/**
cursor_status_t
oadict_equality_next(
	dict_cursor_t 	*cursor,
	ion_value_t		value
)
{
	// @todo if the collection changes, then the status of the cursor needs to change
	oadict_equality_cursor_t *eq_cursor = (oadict_equality_cursor_t *)cursor;
	//check the status of the cursor and if it is not valid or at the end, just exit
	if (eq_cursor->super.status == cs_cursor_unitialized)
		return eq_cursor->super.status;
	else if (eq_cursor->super.status == cs_end_of_results)
		return eq_cursor->super.status;

	//materialize the result
	int idx = eq_cursor->cursor_info->current;


	//this will access map
	((hashmap_t*)(eq_cursor->super.dictionary->instance))->entry;
	//and find the next result? or do this after?

	//and if there are no more results, set the status to reflect
}
*/
/*boolean_t
is_equal(dictionary_t *, ion_key_t *)
{

}*/
