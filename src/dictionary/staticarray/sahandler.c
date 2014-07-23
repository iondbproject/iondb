/******************************************************************************/
/**
@file 		sahandler.c
@author		Raffi Kudlac
@brief		The handler for the static array
 */
/******************************************************************************/

#include "sahandler.h"


status_t
sadict_init(
	dictionary_handler_t 	*handler
)
{
	handler->insert 			= sadict_insert;
	handler->create_dictionary 	= sadict_create;
	handler->get 				= sadict_query;
	handler->update 			= sadict_update;
	handler->find 				= sadict_find;
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
	return sa_get(sa, key, value);
}

status_t
sadict_insert(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_key_t		value
)
{
	static_array_t *sa = (static_array_t *)dictionary->instance;
	return sa_insert(sa, key, value);
}

status_t
sadict_create(
		key_type_t				key_type,
		int 					key_size,
		int 					value_size,
		int 					dictionary_size,
		char				(*compare)(ion_key_t, ion_key_t, ion_key_size_t),
		dictionary_handler_t 	*handler,
		dictionary_t 			*dictionary
)
{
	static_array_t *st;

	st = malloc(sizeof(static_array_t));
	st->key_type = key_type;
	dictionary->instance = st;
	dictionary->handler  = handler;

	return sa_dictionary_create(st, key_size, value_size, dictionary_size, compare);
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
	dictionary->handler  = NULL;

	return result;
}

status_t
sadict_update(
		dictionary_t 	*dictionary,
		ion_key_t 		key,
		ion_value_t 	value
)
{
	static_array_t *sa = (static_array_t *)dictionary->instance;
	return sa_update(sa, key, value);
}

void
sadict_destroy_cursor(
	dict_cursor_t	 **cursor
)
{
	/** Free any internal memory allocations */
	switch((*cursor)->type)
	{
		case predicate_equality:
		{

			free((*cursor)->predicate.statement.equality.equality_value);
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
	free(*cursor);
	*cursor = NULL;
}

cursor_status_t
sadict_equality_next(
	dict_cursor_t 	*cursor,
	ion_value_t		value
)
{
	ion_value_t temp;

	// @todo if the collection changes,
	//then the status of the cursor needs to change
		sadict_cursor_t *sadict_cursor = (sadict_cursor_t *)cursor;

		//check the status of the cursor and if it is not valid or at the end
		// just exit
		if (cursor->status == cs_cursor_uninitialized)
			return cursor->status;
		else if (cursor->status == cs_end_of_results)
			return cursor->status;
		else if ((cursor->status == cs_cursor_initialized )
					|| (cursor->status == cs_cursor_active))
		{
			//in the static array there can only be one key
			static_array_t * sa = cursor->dictionary->instance;
			sa_get(sa, (unsigned char *) &sadict_cursor->current, &temp); //ask scott
			printf("The found info is %s \n", temp);

			memcpy(value, temp, sa->value_size);
			cursor->status = cs_end_of_results;

			return cursor->status;
		}
		return cs_invalid_cursor;
}

status_t
sadict_find(
		dictionary_t			*dictionary,
		predicate_t				*pred,
		dict_cursor_t			**cursor
)
{

	static_array_t *sa = (static_array_t *)dictionary->instance;
	switch(pred->type)
	{
		case predicate_equality:
		{
			if ((*cursor = malloc(sizeof(sadict_cursor_t))) == NULL)
			{
				return err_out_of_memory;
			}

			unsigned char *key = pred->statement.equality.equality_value;
			//initalize stuff
			(*cursor)->dictionary 	= dictionary;
			(*cursor)->type 		= pred->type;
			(*cursor)->status 		= cs_cursor_uninitialized;
			unsigned char *value;

			(*cursor)->destroy 		= sadict_destroy_cursor;

			//as this is an equality, need to malloc for key as well
			if (((*cursor)->predicate.statement.equality.equality_value
					= (ion_key_t)
					malloc(sizeof((((static_array_t*)dictionary->instance)
							->key_size)))) == NULL)
			{
				return err_out_of_memory;
			}

			memcpy((*cursor)->predicate.statement.equality.equality_value,
					pred->statement.equality.equality_value,
					(((static_array_t*)dictionary->instance)->key_size));

			long long k = key_to_index(
					(*cursor)->predicate.statement.equality.equality_value,
					(((static_array_t*)dictionary->instance)->key_size));

			if(k >= sa->maxelements || k < 0) //if the key is too small
			{
				(*cursor)->status = cs_end_of_results;//scott did this
				return status_incorrect_keysize;
			}
			// The requested location is empty
			else if(sa_get(sa, key, &value) == status_empty_slot)
			{
				(*cursor)->status = cs_end_of_results;//scott did this
				return status_empty_slot;
			}
			else
			{
				//setting up more information
				(*cursor)->status = cs_cursor_initialized;
				(*cursor)->next = sadict_equality_next;

				sadict_cursor_t *sadict_cursor = (sadict_cursor_t *)(*cursor);
				sadict_cursor->current = k;
				sadict_cursor->first = k;
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
			return err_invalid_predicate;
			break;
		}
	}
	return status_ok;

}
