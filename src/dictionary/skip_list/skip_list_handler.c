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
	ion_dictionary_handler_t *handler
) {
	handler->insert				= sldict_insert;
	handler->get				= sldict_query;
	handler->create_dictionary	= sldict_create_dictionary;
	handler->remove				= sldict_delete;
	handler->delete_dictionary	= sldict_delete_dictionary;
	handler->update				= sldict_update;
	handler->find				= sldict_find;
	handler->close_dictionary	= sldict_close_dictionary;
	handler->open_dictionary	= sldict_open_dictionary;
}

ion_err_t
sldict_open_dictionary(
	ion_dictionary_handler_t		*handler,
	ion_dictionary_t				*dictionary,
	ion_dictionary_config_info_t	*config,
	ion_dictionary_compare_t		compare
) {
	return err_not_implemented;
}

ion_err_t
sldict_close_dictionary(
	ion_dictionary_t *dictionary
) {
	return err_not_implemented;
}

ion_status_t
sldict_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return sl_insert((ion_skiplist_t *) dictionary->instance, key, value);
}

ion_status_t
sldict_query(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return sl_query((ion_skiplist_t *) dictionary->instance, key, value);
}

ion_err_t
sldict_create_dictionary(
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	int							key_size,
	int							value_size,
	int							dictionary_size,
	ion_dictionary_compare_t	compare,
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary
) {
	UNUSED(id);

	int pnum, pden;

	dictionary->instance = malloc(sizeof(ion_skiplist_t));

	if (NULL == dictionary->instance) {
		return err_out_of_memory;
	}

	dictionary->instance->compare	= compare;

	pnum							= 1;
	pden							= 4;

	/* TODO Should we handle the possible error code returned by this?
	 * If yes, what sorts of errors does it return? */
	ion_err_t result = sl_initialize((ion_skiplist_t *) dictionary->instance, key_type, key_size, value_size, dictionary_size, pnum, pden);

	if (err_ok == result) {
		dictionary->handler = handler;
	}

	return result;
}

ion_status_t
sldict_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
) {
	return sl_delete((ion_skiplist_t *) dictionary->instance, key);
}

ion_err_t
sldict_delete_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_err_t result = sl_destroy((ion_skiplist_t *) dictionary->instance);

	free(dictionary->instance);
	dictionary->instance = NULL;
	return result;
}

ion_status_t
sldict_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return sl_update((ion_skiplist_t *) dictionary->instance, key, value);
}

ion_err_t
sldict_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
) {
	*cursor = malloc(sizeof(ion_sldict_cursor_t));

	ion_skiplist_t *skip_list = (ion_skiplist_t *) dictionary->instance;

	if (NULL == *cursor) {
		return err_out_of_memory;
	}

	(*cursor)->dictionary	= dictionary;
	(*cursor)->status		= cs_cursor_uninitialized;

	(*cursor)->destroy		= sldict_destroy_cursor;
	(*cursor)->next			= sldict_next;

	(*cursor)->predicate	= malloc(sizeof(ion_predicate_t));

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

			ion_sl_node_t *loc = sl_find_node((ion_skiplist_t *) dictionary->instance, target_key);

			if ((NULL == loc->key) || (dictionary->instance->compare(loc->key, target_key, key_size) != 0)) {
				/* If this happens, that means the target key doesn't exist */
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else {
				(*cursor)->status = cs_cursor_initialized;

				ion_sldict_cursor_t *sl_cursor = (ion_sldict_cursor_t *) (*cursor);

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
			ion_sl_node_t *loc = sl_find_node((ion_skiplist_t *) dictionary->instance, (*cursor)->predicate->statement.range.upper_bound);

			if ((NULL == loc->key) || (dictionary->instance->compare(loc->key, (*cursor)->predicate->statement.range.lower_bound, key_size) < 0)) {
				/* This means the returned node is smaller than the lower bound, which means that there are no valid records to return */
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else {
				loc = sl_find_node((ion_skiplist_t *) dictionary->instance, (*cursor)->predicate->statement.range.lower_bound);

				if (NULL == loc->key) {
					/* If this happens, then we hit the head node. Just move to the first valid data item (if exists) */
					loc = loc->next[0];
				}

				/* Increment the location until we hit valid data. It is impossible to fall through here, since we just confirmed previously */
				/* that there does indeed exist valid data (See above check). */
				while (NULL != loc && (dictionary->instance->compare(loc->key, (*cursor)->predicate->statement.range.lower_bound, key_size) < 0)) {
					loc = loc->next[0];
				}

				/* We sanity check this anyways just in case. */
				if (NULL == loc) {
					(*cursor)->status = cs_end_of_results;
					return err_ok;
				}

				(*cursor)->status = cs_cursor_initialized;

				ion_sldict_cursor_t *sl_cursor = (ion_sldict_cursor_t *) (*cursor);

				sl_cursor->current = loc;
				return err_ok;
			}

			break;
		}

		case predicate_all_records: {
			ion_sldict_cursor_t *sl_cursor = (ion_sldict_cursor_t *) (*cursor);

			if (NULL == skip_list->head->next[0]) {
				(*cursor)->status = cs_end_of_results;
			}
			else {
				sl_cursor->current	= skip_list->head->next[0];
				(*cursor)->status	= cs_cursor_initialized;
			}

			return err_ok;
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

ion_cursor_status_t
sldict_next(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
) {
	ion_sldict_cursor_t *sl_cursor = (ion_sldict_cursor_t *) cursor;

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
	ion_dict_cursor_t **cursor
) {
	(*cursor)->predicate->destroy(&(*cursor)->predicate);
	free(*cursor);
	*cursor = NULL;
}

ion_boolean_t
sldict_test_predicate(
	ion_dict_cursor_t	*cursor,
	ion_key_t			key
) {
	ion_skiplist_t	*skiplist	= (ion_skiplist_t *) cursor->dictionary->instance;
	ion_key_size_t	key_size	= cursor->dictionary->instance->record.key_size;
	ion_boolean_t	result		= boolean_false;

	switch (cursor->predicate->type) {
		case predicate_equality: {
			if (skiplist->super.compare(key, cursor->predicate->statement.equality.equality_value, cursor->dictionary->instance->record.key_size) == 0) {
				result = boolean_true;
			}

			break;
		}

		case predicate_range: {
			ion_key_t	lower_b			= cursor->predicate->statement.range.lower_bound;
			ion_key_t	upper_b			= cursor->predicate->statement.range.upper_bound;

			/* Check if key >= lower bound */
			ion_boolean_t comp_lower	= skiplist->super.compare(key, lower_b, key_size) >= 0;

			/* Check if key <= upper bound */
			ion_boolean_t comp_upper	= skiplist->super.compare(key, upper_b, key_size) <= 0;

			result = comp_lower && comp_upper;
			break;
		}

		case predicate_all_records: {
			return boolean_true;
		}
	}

	return result;
}
