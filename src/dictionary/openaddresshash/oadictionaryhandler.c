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
	handler->find 				= oadict_find;
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
	return oah_insert((hashmap_t *)dictionary->instance, key, value);
}

err_t
oadict_query(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t		*value
)
{
	return oah_query((hashmap_t *)dictionary->instance, key, value);
}


err_t
oadict_create_dictionary(
		key_type_t				key_type,
		int 					key_size,
		int 					value_size,
		int 					dictionary_size,
		char					(* compare)(ion_key_t, ion_key_t, ion_key_size_t),
		dictionary_handler_t 	*handler,
		dictionary_t 			*dictionary
)
{
	//this is the instance of the hashmap
	dictionary->instance = (hashmap_t *)malloc(sizeof(hashmap_t));

	//this registers the dictionarys the dictionary
	oah_initialize(
		(hashmap_t *)dictionary->instance,
		oah_compute_simple_hash,
		compare,
		key_type,
		key_size,
		value_size,
		dictionary_size
	);    			// just pick an arbitary size for testing atm

	/**@TODO The correct comparison operator needs to be bound at run time
	 * based on the type of key defined
	 */



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
	return oah_delete((hashmap_t *)dictionary->instance, key);
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
	return oah_update((hashmap_t *)dictionary->instance, key, value);
}

/** @todo What do we do if the cursor is already active? */
err_t
oadict_find(
		dictionary_t 	*dictionary,
		predicate_t 	*predicate,
		dict_cursor_t 	**cursor
)
{
	//based on the type of predicate that is being used, need to create the correct cursor
	switch(predicate->type)
	{
		case predicate_equality:
		{
			//allocate memory for cursor
			if ((*cursor = (dict_cursor_t *)malloc(sizeof(oadict_cursor_t))) == NULL)
			{
				return err_out_of_memory;
			}

			(*cursor)->dictionary = dictionary;
			(*cursor)->type = predicate->type;				//* types align
			(*cursor)->status = cs_cursor_uninitialized;
			//bind destory method for cursor

			(*cursor)->destroy = oadict_destroy_cursor;

			//as this is an equality, need to malloc for key as well
			if (((*cursor)->predicate.statement.equality.equality_value = (ion_key_t)malloc(sizeof((((hashmap_t*)dictionary->instance)->record.key_size)))) == NULL)
			{
				return err_out_of_memory;
			}
			//copy across the key value as the predicate may be destroyed
			memcpy((*cursor)->predicate.statement.equality.equality_value, 	predicate->statement.equality.equality_value, (((hashmap_t*)dictionary->instance)->record.key_size));

			//find the location of the first element as this is a straight equality
			int location = cs_invalid_index;

			//bind correct next function
			(*cursor)->next = oadict_equality_next;	// this will use the correct value

			if (oah_find_item_loc((hashmap_t*)dictionary->instance, (*cursor)->predicate.statement.equality.equality_value, &location) == err_item_not_found)
			{
				//this will still have to be returned?
				(*cursor)->status = cs_end_of_results;
				/*//bind equality function
				eq_cursor->equal = is_equal;*/
				return err_ok;
			}
			else
			{
				(*cursor)->status = cs_cursor_initialized;
				//cast to specific instance type for conveniences of setup
				oadict_cursor_t *oadict_cursor = (oadict_cursor_t *)(* cursor);
				// the cursor is ready to be consumed
				oadict_cursor->first = location;
				oadict_cursor->current = location;
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
		{
			return err_invalid_predicate;		//* Invalid predicate supplied
			break;
		}
	}
	return err_ok;
}

cursor_status_t
oadict_equality_next(
	dict_cursor_t 	*cursor,
	ion_value_t		value
)
{
	// @todo if the collection changes, then the status of the cursor needs to change
	oadict_cursor_t *oadict_cursor = (oadict_cursor_t *)cursor;

	//check the status of the cursor and if it is not valid or at the end, just exit
	if (cursor->status == cs_cursor_uninitialized)
		return cursor->status;
	else if (cursor->status == cs_end_of_results)
		return cursor->status;
	else if ((cursor->status == cs_cursor_initialized )
				|| (cursor->status == cs_cursor_active))	//cursor is active and results have never been accessed
	{

		//materialize the result
		//int idx = oadict_cursor->current;		//this is the current value to return

		//extract reference to map
		hashmap_t *hash_map 	= ((hashmap_t*)cursor->dictionary->instance);

		//assume that the value has been pre-allocated
		//compute length of data record stored in map
		int data_length 		= hash_map->record.key_size
					        				+ hash_map->record.value_size;

		if (cursor->status == cs_cursor_active)							//need to find the next valid entry
		{
			//scan and determine what to do?
			if (cs_end_of_results == oah_scan(oadict_cursor))
			{
				//Then this is the end and there are no more results
				cursor->status = cs_end_of_results;
				/** @todo need to do something with cursor? - done? */
				return cursor->status;
			}
		}
		else
		{
			//if the cursor is initialized but not active, then just read the data and set cursor active
			cursor->status = cs_cursor_active;
		}

		//the results are now ready //reference item at given position
		hash_bucket_t * item	= (((hash_bucket_t *)((hash_map->entry
				        				+ (data_length + SIZEOF(STATUS)) * oadict_cursor->current/*idx*/))));

		//and copy value in
		memcpy(value, (ion_value_t)(item->data+hash_map->record.key_size), hash_map->record.value_size);

		//and update current cursor position
		//oadict_cursor->current = idx;
		return cursor->status;
	}
	//and if you get this far, the cursor is invalid
	return cs_invalid_cursor;
}

boolean_t
is_equal(
	dictionary_t 	*dict,
	ion_key_t 		key1,
	ion_key_t 		key2
)
{
	if (memcmp(key1, key2, ((hashmap_t *)(dict->instance))->record.key_size) == IS_EQUAL)
		return true;
	else
		return false;
}

void
oadict_destroy_cursor(
	dict_cursor_t	 *cursor
)
{
	/** Free any internal memory allocations */
	switch(cursor->type)
	{
		case predicate_equality:
		{

			free(cursor->predicate.statement.equality.equality_value);
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
	}
	/** and free cursor pointer */
	free(cursor);
	cursor = NULL;
}


