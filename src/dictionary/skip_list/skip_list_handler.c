/******************************************************************************/
/**
@file		skip_list_handler.c
@author		Eric Huang
@brief		Handler liaison between dictionary API and skiplist implementation
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#include "skip_list_handler.h"

/**
@brief	  Queries a dictionary instance for a given @p key and returns the
			corresponding @p value.

@details	Queries a dictionary instance for a given @p key and returns the
			@p value within, copied into the pointer provided by the user.
			Assumption is that the pointer is passed unallocated, for this
			function to allocate. The responsibility is then on the user to
			free the given memory.

@param	  dictionary
				The instance of the dictionary to query
@param	  key
				The key to search for.
@param	  value
				A pointer used to hold the returned value from the query. The
				memory for value is assumed to be allocated and freed by the
				user.
@return	 Status of query.
*/
ion_status_t
sldict_get(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return sl_get((ion_skiplist_t *) dictionary->instance, key, value);
}

/**
@brief	  Next function queries and retrieves the next key/value pair that
			satisfies the predicate of the cursor.

@param	  cursor
				The cursor used to iterate over results.
@param	  record
				A record pointer that is allocated by the caller in which the
				cursor will fill with the next key/value result. The assumption
				is that the caller will also free this memory.
@return	 Status of cursor.
*/
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
			if ((NULL == sl_cursor->current) || (test_predicate(cursor, sl_cursor->current->key) == boolean_false)) {
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

/**
@brief			Closes a skiplist instance of a dictionary.

@param			dictionary
					A pointer to the specific dictionary instance to be closed.

@return			The status of closing the dictionary.
 */
ion_err_t
sldict_close_dictionary(
	ion_dictionary_t *dictionary
) {
	UNUSED(dictionary);
	return err_not_implemented;
}

/**
@brief	  Destroys the cursor.

@details	Destroys the cursor when the user is finished with it. All memory
			internally used by the cursor is freed as well. Cursor pointers
			will be set to NULL as per IonDB specification.

@param	  cursor
				Pointer to a pointer of a cursor.
*/
void
sldict_destroy_cursor(
	ion_dict_cursor_t **cursor
) {
	(*cursor)->predicate->destroy(&(*cursor)->predicate);
	free(*cursor);
	*cursor = NULL;
}

/**
@brief	  Finds multiple keys based on the provided predicate.

@details	Finds multiple keys based on the provided predicate. Gives a cursor
			that allows traversal of all key/value pairs that satisfy the
			@p predicate. Not all implementations support a find.

@param	  dictionary
				The instance of a dictionary to search within.
@param	  predicate
				The predicate used to match.
@param	  cursor
				The pointer to a cursor declared by the caller, but initialized
				and populated within the function.
@return	 Status of find.
*/
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
			break;
		}

		default: {
			return err_invalid_predicate;
			break;
		}
	}

	return err_ok;
}

/**
@brief			Opens a specific skiplist instance of a dictionary.

@param			handler
					A pointer to the handler for the specific dictionary being opened.
@param			dictionary
					The pointer declared by the caller that will reference
					the instance of the dictionary opened.
@param			config
					The configuration info of the specific dictionary to be opened.
@param			compare
					Function pointer for the comparison function for the dictionary.

@return			The status of opening the dictionary.
 */
ion_err_t
sldict_open_dictionary(
	ion_dictionary_handler_t		*handler,
	ion_dictionary_t				*dictionary,
	ion_dictionary_config_info_t	*config,
	ion_dictionary_compare_t		compare
) {
	UNUSED(handler);
	UNUSED(dictionary);
	UNUSED(config);
	UNUSED(compare);
	return err_not_implemented;
}

void
sldict_init(
	ion_dictionary_handler_t *handler
) {
	handler->insert				= sldict_insert;
	handler->get				= sldict_get;
	handler->create_dictionary	= sldict_create_dictionary;
	handler->remove				= sldict_delete;
	handler->delete_dictionary	= sldict_delete_dictionary;
	handler->destroy_dictionary = sldict_destroy_dictionary;
	handler->update				= sldict_update;
	handler->find				= sldict_find;
	handler->close_dictionary	= sldict_close_dictionary;
	handler->open_dictionary	= sldict_open_dictionary;
}

ion_status_t
sldict_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return sl_insert((ion_skiplist_t *) dictionary->instance, key, value);
}

ion_err_t
sldict_create_dictionary(
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size,
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
	dictionary->instance->type		= dictionary_type_skip_list_t;

	pnum							= 1;
	pden							= 4;

	ion_err_t result = sl_initialize((ion_skiplist_t *) dictionary->instance, key_type, key_size, value_size, dictionary_size, pnum, pden);

	if ((err_ok == result) && (NULL != handler)) {
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

ion_err_t
sldict_destroy_dictionary(
	ion_dictionary_id_t id
) {
	UNUSED(id);
	return err_not_implemented;
}

ion_status_t
sldict_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return sl_update((ion_skiplist_t *) dictionary->instance, key, value);
}
