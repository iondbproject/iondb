/******************************************************************************/
/**
 @file 		oadictionaryhandler.c
 @author		Scott Ronald Fazackerley
 @brief		The handler for a hash table using linear probing.
 */
/******************************************************************************/

#include "oadictionaryhandler.h"
#include "kv_system.h"

void oadict_init(dictionary_handler_t *handler)
{
	handler->insert = oadict_insert;
	handler->create_dictionary = oadict_create_dictionary;
	handler->get = oadict_query;
	handler->update = oadict_update;
	handler->find = oadict_find;
	handler->remove = oadict_delete;
	handler->delete_dictionary = oadict_delete_dictionary;
}

err_t oadict_insert(dictionary_t *dictionary, ion_key_t key, ion_value_t value)
{
	return oah_insert((hashmap_t *)dictionary->instance, key, value);
}

err_t oadict_query(dictionary_t *dictionary, ion_key_t key, ion_value_t value)
{
	return oah_query((hashmap_t *)dictionary->instance, key, value);
}

err_t oadict_create_dictionary(
    key_type_t key_type,
    int key_size,
    int value_size,
    int dictionary_size,
    char (*compare)(ion_key_t, ion_key_t, ion_key_size_t),
    dictionary_handler_t *handler,
    dictionary_t *dictionary)
{
	//this is the instance of the hashmap
	dictionary->instance = (dictionary_parent_t *)malloc(sizeof(hashmap_t));

	dictionary->instance->compare = compare;

	//this registers the dictionary the dictionary
	oah_initialize((hashmap_t *)dictionary->instance, oah_compute_simple_hash,
	        key_type, key_size, value_size, dictionary_size); // just pick an arbitary size for testing atm

	/**@TODO The correct comparison operator needs to be bound at run time
	 * based on the type of key defined
	 */

	//register the correct handler
	dictionary->handler = handler;//todo: need to check to make sure that the handler is registered

	return 0;
}

err_t oadict_delete(dictionary_t *dictionary, ion_key_t key)
{
	return oah_delete((hashmap_t *)dictionary->instance, key);
}

err_t oadict_delete_dictionary(dictionary_t *dictionary)
{
	err_t result = oah_destroy((hashmap_t *)dictionary->instance);
	free(dictionary->instance);
	dictionary->instance = NULL;// When releasing memory, set pointer to NULL
	return result;
}

err_t oadict_update(dictionary_t *dictionary, ion_key_t key, ion_value_t value)
{
	return oah_update((hashmap_t *)dictionary->instance, key, value);
}

/** @todo What do we do if the cursor is already active? */
err_t oadict_find(
    dictionary_t *dictionary,
    predicate_t *predicate,
    dict_cursor_t **cursor)
{

	//allocate memory for cursor
	if ((*cursor = (dict_cursor_t *)malloc(sizeof(oadict_cursor_t))) == NULL)
	{
		return err_out_of_memory;
	}
	(*cursor)->dictionary = dictionary;
	(*cursor)->type = predicate->type;				//* types align
	(*cursor)->status = cs_cursor_uninitialized;

	//bind destroy method for cursor
	(*cursor)->destroy = oadict_destroy_cursor;

	//bind correct next function
	(*cursor)->next = oadict_next;	// this will use the correct value

	//allocate predicate
	(*cursor)->predicate = (predicate_t *)malloc(sizeof(predicate_t));
	(*cursor)->predicate->type = predicate->type; /**@todo repair as there are duplicate types */

	//based on the type of predicate that is being used, need to create the correct cursor
	switch (predicate->type)
	{
		case predicate_equality:
		{
			//as this is an equality, need to malloc for key as well
			if (((*cursor)->predicate->statement.equality.equality_value =
			        (ion_key_t)malloc(
			                (((hashmap_t*)dictionary->instance)->super.record.key_size)))
			        == NULL)
			{
				free((*cursor)->predicate);
				free(*cursor);						//cleanup
				return err_out_of_memory;
			}
			//copy across the key value as the predicate may be destroyed
			memcpy((*cursor)->predicate->statement.equality.equality_value,
			        predicate->statement.equality.equality_value,
			        ((((hashmap_t*)dictionary->instance)->super.record.key_size)));

			//find the location of the first element as this is a straight equality
			int location = cs_invalid_index;

			if (oah_find_item_loc((hashmap_t*)dictionary->instance,
			        (*cursor)->predicate->statement.equality.equality_value,
			        &location) == err_item_not_found)
			{
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else
			{
				(*cursor)->status = cs_cursor_initialized;
				//cast to specific instance type for conveniences of setup
				oadict_cursor_t *oadict_cursor = (oadict_cursor_t *)(*cursor);
				// the cursor is ready to be consumed
				oadict_cursor->first = location;

				oadict_cursor->current = location;
				return err_ok;
			}
			break;
		}
		case predicate_range:
		{
			//as this is a range, need to malloc leq key
			if (((*cursor)->predicate->statement.range.leq_value =
			        (ion_key_t)malloc((((hashmap_t*)dictionary->instance)->super.record.key_size)))
			        == NULL)
			{
				free((*cursor)->predicate);
				free(*cursor);					//cleanup
				return err_out_of_memory;
			}
			//copy across the key value as the predicate may be destroyed
			memcpy((*cursor)->predicate->statement.range.leq_value,
			        predicate->statement.range.leq_value,
			        (((hashmap_t *)dictionary->instance)->super.record.key_size));

			//as this is a range, need to malloc leq key
			if (((*cursor)->predicate->statement.range.geq_value =
			        (ion_key_t)malloc(
			        		(((hashmap_t*)dictionary->instance)->super.record.key_size)))
			        == NULL)
			{
				free((*cursor)->predicate->statement.range.leq_value);
				free((*cursor)->predicate);
				free(*cursor);					//cleanup
				return err_out_of_memory;
			}
			//copy across the key value as the predicate may be destroyed
			memcpy((*cursor)->predicate->statement.range.geq_value,
			        predicate->statement.range.geq_value,
			        (((hashmap_t*)dictionary->instance)->super.record.key_size));

			//find the location of the first element as this is a straight equality
			int location = cs_invalid_index;

			//start at the lowest end of the range and check
			if (oah_find_item_loc((hashmap_t*)dictionary->instance,
			        (*cursor)->predicate->statement.range.geq_value, &location)
			        == err_item_not_found)
			{
				//this will still have to be returned?
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else
			{
				(*cursor)->status = cs_cursor_initialized;
				//cast to specific instance type for conveniences of setup
				oadict_cursor_t *oadict_cursor = (oadict_cursor_t *)(*cursor);
				// the cursor is ready to be consumed
				oadict_cursor->first = location;
				oadict_cursor->current = location;
				return err_ok;
			}
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

cursor_status_t oadict_next(
	dict_cursor_t 	*cursor,
	ion_record_t 	*record
)
{
	// @todo if the collection changes, then the status of the cursor needs to change
	oadict_cursor_t *oadict_cursor = (oadict_cursor_t *)cursor;

	//check the status of the cursor and if it is not valid or at the end, just exit
	if (cursor->status == cs_cursor_uninitialized)
		return cursor->status;
	else if (cursor->status == cs_end_of_results)
		return cursor->status;
	else if ((cursor->status == cs_cursor_initialized)
	        || (cursor->status == cs_cursor_active))//cursor is active and results have never been accessed
	{
		//extract reference to map
		hashmap_t *hash_map = ((hashmap_t*)cursor->dictionary->instance);

		//assume that the value has been pre-allocated
		//compute length of data record stored in map
		int data_length = hash_map->super.record.key_size
		        + hash_map->super.record.value_size;

		if (cursor->status == cs_cursor_active)		//find the next valid entry
		{
			//scan and determine what to do?
			if (cs_end_of_results == oadict_scan(oadict_cursor))
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
		hash_bucket_t * item =
		        (((hash_bucket_t *)((hash_map->entry
		                + (data_length + SIZEOF(STATUS))
		                        * oadict_cursor->current/*idx*/))));


		/** @todo A discussion needs to be had regarding ion_record_t and its format in memory etc */
		//and copy key and value in

		memcpy(record->key,
				(ion_key_t)(item->data),
				hash_map->super.record.key_size);

		memcpy(record->value,
		        (ion_value_t)(item->data + hash_map->super.record.key_size),
		        hash_map->super.record.value_size);

		//and update current cursor position
		return cursor->status;
	}
	//and if you get this far, the cursor is invalid
	return cs_invalid_cursor;
}

boolean_t is_equal(dictionary_t *dict, ion_key_t key1, ion_key_t key2)
{
	if (memcmp(key1, key2,
	        (((hashmap_t*)dict->instance)->super.record.key_size)) == IS_EQUAL)
		return boolean_true;
	else
		return boolean_false;
}

void oadict_destroy_cursor(dict_cursor_t **cursor)
{
	/** Free any internal memory allocations */
	switch ((*cursor)->type)
	{
		case predicate_equality:
		{
			free((*cursor)->predicate->statement.equality.equality_value);
			break;
		}
		case predicate_range:
		{
			free((*cursor)->predicate->statement.range.geq_value);
			free((*cursor)->predicate->statement.range.leq_value);
			break;
		}
		case predicate_predicate:
		{
			break;
		}
	}
	/** and free cursor pointer */
	free((*cursor)->predicate);
	free(*cursor);
	*cursor = NULL;
}

boolean_t oadict_test_predicate(dict_cursor_t *cursor, ion_key_t key)
{
	//need to check key match; what's the most efficient way?
	//need to use fnptr here for the correct matching
	/*int key_is_eq ual = memcmp(item->data, "10{" ,
	 hash_map->record.key_size);*/
	/**
	 * Compares value == key
	 */
	int key_satisfies_predicate;
	hashmap_t * hash_map = (hashmap_t *)(cursor->dictionary->instance);

	//pre-prime value for faster exit
	key_satisfies_predicate = boolean_false;

	switch (cursor->type)
	{
		case cursor_equality: //equality scan check
		{
			if (IS_EQUAL
			        == hash_map->super.compare(
			                cursor->predicate->statement.equality.equality_value,
			                key, hash_map->super.record.key_size))
			{
				key_satisfies_predicate = boolean_true;
			}
			break;
		}
		case cursor_range: // range check
		{
			if (		// leq_value <= key <==> !(leq_value > key)
			(!(A_gt_B
			        == hash_map->super.compare(key,
			                cursor->predicate->statement.range.leq_value,
			                hash_map->super.record.key_size))) &&// key <= geq_value <==> !(key > geq_key)
			        (!(A_gt_B
			                == hash_map->super.compare(
			                        cursor->predicate->statement.range.geq_value,
			                        key, hash_map->super.record.key_size))))
			{
				key_satisfies_predicate = boolean_true;
			}
			break;
		}
	}
	return key_satisfies_predicate;
}

err_t oadict_scan(oadict_cursor_t *cursor//know exactly what implementation of cursor is
    )
{
	//need to scan hashmap fully looking for values that satisfy - need to think about
	hashmap_t * hash_map = (hashmap_t *)(cursor->super.dictionary->instance);

	int loc = (cursor->current + 1) % hash_map->map_size;
	//this is the current position of the cursor
	//and start scanning 1 ahead

	//start at the current position, scan forward
	while (loc != cursor->first)
	{
		// check to see if current item is a match based on key
		// locate first item
		hash_bucket_t * item = (((hash_bucket_t *)((hash_map->entry
		        + (hash_map->super.record.key_size
		                + hash_map->super.record.value_size + SIZEOF(STATUS))
		                * loc))));

		if (item->status == EMPTY || item->status == DELETED)
		{
			//if empty, just skip to next cell
			loc++;
		}
		else //check to see if the current key value satisfies the predicate
		{

			//need to check key match; what's the most efficient way?
			//need to use fnptr here for the correct matching
			/*int key_is_equal = memcmp(item->data, "10{" ,
			 hash_map->record.key_size);*/
			/**
			 * Compares value == key
			 */
			boolean_t key_satisfies_predicate = oadict_test_predicate(
			        &(cursor->super), (ion_key_t)item->data);//assumes that the key is first

			if (key_satisfies_predicate == boolean_true)
			{
				cursor->current = loc;		//this is the next index for value
				return cs_valid_data;
			}
		}
		loc++;
		if (loc >= hash_map->map_size)	// Perform wrapping
			loc = 0;
	}
	//if you end up here, you've wrapped the entire data structure and not found a value
	return cs_end_of_results;
}
