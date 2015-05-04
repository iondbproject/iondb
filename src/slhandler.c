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
	handler->remove 			= sldict_delete;
	handler->delete_dictionary 	= sldict_delete_dictionary;
	handler->update 			= sldict_update;
	handler->find 				= sldict_find;

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
	ion_value_t 	value
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

	dictionary->instance 			= malloc(sizeof(skiplist_t));
	if(NULL == dictionary->instance) { return err_out_of_memory; }

	dictionary->instance->compare 	= compare;

	pnum 							= 1;
	pden 							= 4;

	/* TODO Should we handle the possible error code returned by this?
	 * If yes, what sorts of errors does it return? */
	err_t result = sl_initialize(
							(skiplist_t *) dictionary->instance,
							key_type,
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
	if(NULL == (*cursor)->predicate)
	{
		free(*cursor);
		return err_out_of_memory;
	}
	(*cursor)->predicate->type 	= predicate->type;


	ion_key_size_t 	key_size 	= dictionary->instance->record.key_size;

	switch(predicate->type)
	{
		case predicate_equality:
		{
			/* TODO get ALL these lines within 80 cols */
			ion_key_t 		target_key 	= predicate->statement.equality.equality_value;

			(*cursor)->predicate->statement.equality.equality_value = malloc(key_size);
			if(NULL == (*cursor)->predicate->statement.equality.equality_value)
			{
				free( (*cursor)->predicate);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy(
				(*cursor)->predicate->statement.equality.equality_value,
				target_key,
				key_size
			);

			sl_node_t *loc = sl_find_node(
								(skiplist_t *) dictionary->instance,
								target_key
							 );
			if(NULL == loc->key || dictionary->instance->compare(loc->key, target_key, key_size) != 0)
			{
				/* If this happens, that means the target key doesn't exist */
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else
			{
				(*cursor)->status 			= cs_cursor_initialized;
				sldict_cursor_t *sl_cursor 	= (sldict_cursor_t *) (*cursor);
				sl_cursor->current 			= loc;
				return err_ok;
			}

			break;
		}
		case predicate_range:
		{
			(*cursor)->predicate->statement.range.leq_value = malloc(key_size);
			if(NULL == (*cursor)->predicate->statement.range.leq_value)
			{
				free( (*cursor)->predicate);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy(
				(*cursor)->predicate->statement.range.leq_value,
				predicate->statement.range.leq_value,
				key_size
			);

			(*cursor)->predicate->statement.range.geq_value = malloc(key_size);
			if(NULL == (*cursor)->predicate->statement.range.geq_value)
			{
				free((*cursor)->predicate->statement.range.leq_value);
				free((*cursor)->predicate);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy(
				(*cursor)->predicate->statement.range.geq_value,
				predicate->statement.range.geq_value,
				key_size
			);

			sl_node_t *loc = sl_find_node(
								(skiplist_t *) dictionary->instance,
								(*cursor)->predicate->statement.range.leq_value
							 );
			if(NULL == loc->key ||
							dictionary->instance->compare(
								loc->key,
								(*cursor)->predicate->statement.range.leq_value,
								key_size
							) < 0)
			/* This means the returned node is smaller than the lower bound */
			{
				(*cursor)->status 	= cs_end_of_results;
				return err_ok;
			}
			else
			{
				(*cursor)->status 			= cs_cursor_initialized;
				sldict_cursor_t *sl_cursor 	= (sldict_cursor_t *) (*cursor);
				sl_cursor->current 			= loc;
				return err_ok;
			}
			break;
		}
		case predicate_predicate:
		{
			/* TODO not implemented */
			break;
		}
		default:
		{
			return err_invalid_predicate;
			break;
		}
	}

	return err_ok;
}

cursor_status_t
sldict_next(
	dict_cursor_t 	*cursor,
	ion_record_t 	*record
)
{
	sldict_cursor_t *sl_cursor 	= (sldict_cursor_t *) cursor;

	if(cursor->status == cs_cursor_uninitialized)
	{
		return cursor->status;
	}
	else if(cursor->status == cs_end_of_results)
	{
		return cursor->status;
	}
	else if(cursor->status == cs_cursor_initialized ||
			cursor->status == cs_cursor_active)
	{
		if(cursor->status == cs_cursor_active)
		{
			if(NULL == sl_cursor->current ||
				sldict_test_predicate(cursor, sl_cursor->current->key) == boolean_false)
			{
				cursor->status 	= cs_end_of_results;
				return cursor->status;
			}
		}
		else /* The status is cs_cursor_initialized */
		{
			cursor->status 	= cs_cursor_active;
		}

		/*Copy both key and value into user provided struct */
		memcpy(
			record->key,
			sl_cursor->current->key,
			cursor->dictionary->instance->record.key_size
		);
		memcpy(
			record->value,
			sl_cursor->current->value,
			cursor->dictionary->instance->record.value_size
		);

		sl_cursor->current 	= sl_cursor->current->next[0];
		return cursor->status;
	}

	return cs_invalid_cursor;
}

/* TODO test me */
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
			/* TODO not implemented yet */
			break;
		}
	}

	free( (*cursor)->predicate);
	free(*cursor);
	*cursor = NULL;
}

boolean_t
sldict_test_predicate(
	dict_cursor_t 	*cursor,
	ion_key_t 		key
)
{
	skiplist_t 		*skiplist 	= (skiplist_t*) cursor->dictionary->instance;
	ion_key_size_t 	key_size	= cursor->dictionary->instance->record.key_size;
	boolean_t 		result 		= boolean_false;

	switch(cursor->type)
	{
		case cursor_equality:
		{
			if(skiplist->super.compare(
						key,
						cursor->predicate->statement.equality.equality_value,
						cursor->dictionary->instance->record.key_size
					) == 0
				)
			{
				result = boolean_true;
			}
			break;
		}
		case cursor_range:
		{
			ion_key_t lower_b 	= cursor->predicate->statement.range.leq_value;
			ion_key_t upper_b 	= cursor->predicate->statement.range.geq_value;

			/* Check if key >= lower bound */
			boolean_t comp_lower = skiplist->super.compare(key, lower_b, key_size) >= 0;

			/* Check if key <= upper bound */
			boolean_t comp_upper = skiplist->super.compare(key, upper_b, key_size) <= 0;

			result = comp_lower && comp_upper;
			break;
		}
	}

	return result;
}
