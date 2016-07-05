/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Handler liaison between dictionary API and skiplist implementation
*/
/******************************************************************************/

#include "skip_list_handler.h"

void
sldict_init(
	dictionary_handler_t *handler
) {
	handler->insert				= sldict_insert;
	handler->get				= sldict_query;
	handler->create_dictionary	= sldict_create_dictionary;
	handler->remove				= sldict_delete;
	handler->delete_dictionary	= sldict_delete_dictionary;
	handler->update				= sldict_update;
	handler->find				= sldict_find;
}

ion_status_t
sldict_insert(
	dictionary_t	*dictionary,
	ion_key_t		key,
	ion_value_t		value
) {
	return sl_insert((skiplist_t *) dictionary->instance, key, value);
}

ion_status_t
sldict_query(
	dictionary_t	*dictionary,
	ion_key_t		key,
	ion_value_t		value
) {
	return sl_query((skiplist_t *) dictionary->instance, key, value);
}

err_t
sldict_create_dictionary(
	ion_dictionary_id_t			id,
	key_type_t					key_type,
	int							key_size,
	int							value_size,
	int							dictionary_size,
	ion_dictionary_compare_t	compare,
	dictionary_handler_t		*handler,
	dictionary_t				*dictionary
) {
	UNUSED(id);

	int pnum, pden;

	dictionary->instance = malloc(sizeof(skiplist_t));

	if (NULL == dictionary->instance) {
		return err_out_of_memory;
	}

	dictionary->instance->compare	= compare;

	pnum							= 1;
	pden							= 4;

	/* TODO Should we handle the possible error code returned by this?
	 * If yes, what sorts of errors does it return? */
	err_t result = sl_initialize((skiplist_t *) dictionary->instance, key_type, key_size, value_size, dictionary_size, pnum, pden);

	if (err_ok == result) {
		dictionary->handler = handler;
	}

	return result;
}

ion_status_t
sldict_delete(
	dictionary_t	*dictionary,
	ion_key_t		key
) {
	return sl_delete((skiplist_t *) dictionary->instance, key);
}

err_t
sldict_delete_dictionary(
	dictionary_t *dictionary
) {
	err_t result = sl_destroy((skiplist_t *) dictionary->instance);

	free(dictionary->instance);
	dictionary->instance = NULL;
	return result;
}

ion_status_t
sldict_update(
	dictionary_t	*dictionary,
	ion_key_t		key,
	ion_value_t		value
) {
	return sl_update((skiplist_t *) dictionary->instance, key, value);
}

err_t
sldict_find(
	dictionary_t	*dictionary,
	predicate_t		*predicate,
	dict_cursor_t	**cursor
) {
	*cursor = malloc(sizeof(sldict_cursor_t));

	if (NULL == *cursor) {
		return err_out_of_memory;
	}

	(*cursor)->dictionary	= dictionary;
	(*cursor)->status		= cs_cursor_uninitialized;

	(*cursor)->destroy		= sldict_destroy_cursor;
	(*cursor)->next			= sldict_next;

	(*cursor)->predicate	= malloc(sizeof(predicate_t));

	if (NULL == (*cursor)->predicate) {
		free(*cursor);
		return err_out_of_memory;
	}

	(*cursor)->predicate->type		= predicate->type;
	(*cursor)->predicate->destroy	= predicate->destroy;

	ion_key_size_t key_size = dictionary->instance->record.key_size;

	switch (predicate->type) {
		case predicate_equality: {
			/* TODO get ALL these lines within 80 cols */
			ion_key_t target_key = predicate->statement.equality.equality_value;

			(*cursor)->predicate->statement.equality.equality_value = malloc(key_size);

			if (NULL == (*cursor)->predicate->statement.equality.equality_value) {
				free((*cursor)->predicate);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy((*cursor)->predicate->statement.equality.equality_value, target_key, key_size);

			sl_node_t *loc = sl_find_node((skiplist_t *) dictionary->instance, target_key);

			if ((NULL == loc->key) || (dictionary->instance->compare(loc->key, target_key, key_size) != 0)) {
				/* If this happens, that means the target key doesn't exist */
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else {
				(*cursor)->status = cs_cursor_initialized;

				sldict_cursor_t *sl_cursor = (sldict_cursor_t *) (*cursor);

				sl_cursor->current = loc;
				return err_ok;
			}

			break;
		}

		case predicate_range: {
			(*cursor)->predicate->statement.range.lower_bound = malloc(key_size);

			if (NULL == (*cursor)->predicate->statement.range.lower_bound) {
				free((*cursor)->predicate);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy((*cursor)->predicate->statement.range.lower_bound, predicate->statement.range.lower_bound, key_size);

			(*cursor)->predicate->statement.range.upper_bound = malloc(key_size);

			if (NULL == (*cursor)->predicate->statement.range.upper_bound) {
				free((*cursor)->predicate->statement.range.lower_bound);
				free((*cursor)->predicate);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy((*cursor)->predicate->statement.range.upper_bound, predicate->statement.range.upper_bound, key_size);

			/* Try to find the node containing the upper bound. */
			sl_node_t *loc = sl_find_node((skiplist_t *) dictionary->instance, (*cursor)->predicate->statement.range.upper_bound);

			if ((NULL == loc->key) || (dictionary->instance->compare(loc->key, (*cursor)->predicate->statement.range.lower_bound, key_size) < 0)) {
				/* This means the returned node is smaller than the lower bound, which means that there are no valid records to return */
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else {
				(*cursor)->status = cs_cursor_initialized;

				sldict_cursor_t *sl_cursor = (sldict_cursor_t *) (*cursor);

				sl_cursor->current = loc;
				return err_ok;
			}

			break;
		}

		case predicate_predicate: {
			/* TODO not implemented */
			break;
		}

		default: {
			return err_invalid_predicate;
			break;
		}
	}

	return err_ok;
}

cursor_status_t
sldict_next(
	dict_cursor_t	*cursor,
	ion_record_t	*record
) {
	sldict_cursor_t *sl_cursor = (sldict_cursor_t *) cursor;

	if (cursor->status == cs_cursor_uninitialized) {
		return cursor->status;
	}
	else if (cursor->status == cs_end_of_results) {
		return cursor->status;
	}
	else if ((cursor->status == cs_cursor_initialized) || (cursor->status == cs_cursor_active)) {
		if (cursor->status == cs_cursor_active) {
			if ((NULL == sl_cursor->current) || (sldict_test_predicate(cursor, sl_cursor->current->key) == boolean_false)) {
				cursor->status = cs_end_of_results;
				return cursor->status;
			}
		}
		else {
			/* The status is cs_cursor_initialized */
			cursor->status = cs_cursor_active;
		}

		/*Copy both key and value into user provided struct */
		memcpy(record->key, sl_cursor->current->key, cursor->dictionary->instance->record.key_size);
		memcpy(record->value, sl_cursor->current->value, cursor->dictionary->instance->record.value_size);

		sl_cursor->current = sl_cursor->current->next[0];
		return cursor->status;
	}

	return cs_invalid_cursor;
}

void
sldict_destroy_cursor(
	dict_cursor_t **cursor
) {
	(*cursor)->predicate->destroy(&(*cursor)->predicate);
	free(*cursor);
	*cursor = NULL;
}

boolean_t
sldict_test_predicate(
	dict_cursor_t	*cursor,
	ion_key_t		key
) {
	skiplist_t		*skiplist	= (skiplist_t *) cursor->dictionary->instance;
	ion_key_size_t	key_size	= cursor->dictionary->instance->record.key_size;
	boolean_t		result		= boolean_false;

	switch (cursor->predicate->type) {
		case predicate_equality: {
			if (skiplist->super.compare(key, cursor->predicate->statement.equality.equality_value, cursor->dictionary->instance->record.key_size) == 0) {
				result = boolean_true;
			}

			break;
		}

		case predicate_range: {
			ion_key_t	lower_b		= cursor->predicate->statement.range.lower_bound;
			ion_key_t	upper_b		= cursor->predicate->statement.range.upper_bound;

			/* Check if key >= lower bound */
			boolean_t comp_lower	= skiplist->super.compare(key, lower_b, key_size) >= 0;

			/* Check if key <= upper bound */
			boolean_t comp_upper	= skiplist->super.compare(key, upper_b, key_size) <= 0;

			result = comp_lower && comp_upper;
			break;
		}
	}

	return result;
}
