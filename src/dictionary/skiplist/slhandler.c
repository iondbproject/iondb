/******************************************************************************/
/**
@file		slhandler.c
@author		Kris Wallperington
@brief		Handler liaison between dictionary API and skiplist implementation
*/
/******************************************************************************/

#include "slhandler.h"

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
	return sl_query((skiplist_t *) dictionary->instance, key, value);
}

err_t
sldict_create_dictionary(
	key_type_t 				key_type,
	int 					key_size,
	int 					value_size,
	int 					dictionary_size,
	char 					(*compare)(ion_key_t, ion_key_t, ion_key_size_t),
	dictionary_handler_t 	*handler,
	dictionary_t 			*dictionary
)
{
	int pnum, pden;

	dictionary->instance 	= malloc(sizeof(skiplist_t));

	if(NULL == dictionary->instance) { return err_out_of_memory; }

	pnum 					= 1;
	pden 					= 4;

	/* TODO Should we handle the possible error code returned by this?
	 * If yes, what sorts of errors does it return? */
	err_t result = sl_initialize(
							(skiplist_t *) dictionary->instance,
							key_type,
							compare,
							key_size,
							value_size,
							dictionary_size,
							pnum,
							pden
					);

	if(err_ok == result)
	{
		dictionary->handler 	= handler;
	}

	return result;
}

err_t
sldict_delete(
	dictionary_t 	*dictionary,
	ion_key_t 		key
)
{
	return sl_delete((skiplist_t *) dictionary->instance, key);
}

err_t
sldict_delete_dictionary(
	dictionary_t 	*dictionary
)
{

	err_t result		 = sl_destroy((skiplist_t *) dictionary->instance);
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
	return sl_update((skiplist_t *) dictionary->instance, key, value);
}

err_t
sldict_find(
	dictionary_t 	*dictionary,
	predicate_t 	*predicate,
	dict_cursor_t 	**cursor
)
{
	*cursor 					= malloc(sizeof(sldict_cursor_t));
	if(NULL == *cursor) { return err_out_of_memory; }

	(*cursor)->dictionary 		= dictionary;
	(*cursor)->type 			= predicate->type;
	(*cursor)->status 			= cs_cursor_uninitialized;

	(*cursor)->destroy 			= sldict_destroy_cursor;
	(*cursor)->next 			= sldict_next;

	(*cursor)->predicate 		= malloc(sizeof(predicate_t));
	(*cursor)->predicate->type 	= predicate->type;

	switch(predicate->type)
	{
		case predicate_equality:
		{
			ion_key_size_t key_size = dictionary->instance->record.key_size;
			/* TODO yuck */
			(*cursor)->predicate->statement.equality.equality_value
								= malloc(sizeof(char) * key_size);
			if(NULL == (*cursor)->predicate->statement.equality.equality_value)
			{
				free( (*cursor)->predicate);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy(
				(*cursor)->predicate->statement.equality.equality_value,
				predicate->statement.equality.equality_value,
				key_size
			);

			/* TODO last here, was about to do a find_node */

			break;
		}
	}

	return err_ok;
}

cursor_status_t
sldict_next( /* TODO finish NEXT */
	dict_cursor_t 	*cursor,
	ion_value_t 	value
)
{
	return cs_possible_data_inconsistency;
}

void
sldict_destroy_cursor(
	dict_cursor_t 		**cursor
)
{
	switch( (*cursor)->type)
	{
		case predicate_equality:
		{
			free( (*cursor)->predicate->statement.equality.equality_value);
			break;
		}
		case predicate_range:
		{
			free( (*cursor)->predicate->statement.range.geq_value);
			free( (*cursor)->predicate->statement.range.leq_value);
			break;
		}
		case predicate_predicate:
		{
			/* TODO what's this? */
			break;
		}
	}

	free( (*cursor)->predicate);
	free(*cursor);
	*cursor = NULL;
}
