/******************************************************************************/
/**
@file		open_address_hash_dictionary_handler.c
@author		Scott Ronald Fazackerley
@brief		The handler for a hash table using linear probing.
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

#include "open_address_hash_dictionary_handler.h"

/**
@brief	  Queries a dictionary instance for the given @p key and returns
			the associated @p value.

@details	Queries a dictionary instance for the given @p key and returns
			the associated @p value.  If the @p write_concern is set to
			wc_insert_unique then if the @p key exists already, an error will
			be generated as duplicate keys are prevented.  If the
			@p write_concern is set to wc_update, the updates are allowed.
			In this case, if the @p key exists in the hashmap, the @p value
			will be updated.  If the @p key does not exist, then a new item
			will be inserted to hashmap.

@param	  dictionary
				The instance of the dictionary to query.
@param	  key
				The key to search for.
@param	  value
				A pointer that is used to return the value associated with
				the provided key.  The function will malloc memory for the
				value and it is up to the consumer the free the associated
				memory.
@return	 The status of the query.
*/
ion_status_t
oadict_get(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return oah_get((ion_hashmap_t *) dictionary->instance, key, value);
}

/**

@brief		  Starts scanning map looking for conditions that match
				predicate and returns result.

@details		Scans that map looking for the next value that satisfies the predicate.
				The next valid index is returned through the cursor

@param		  cursor
					A pointer to the cursor that is operating on the map.

@return		 The status of the scan.
*/
ion_err_t
oadict_scan(
	ion_oadict_cursor_t *cursor	/* don't need to pass in the cursor */
) {
	/* need to scan hashmap fully looking for values that satisfy - need to think about */
	ion_hashmap_t *hash_map = (ion_hashmap_t *) (cursor->super.dictionary->instance);

	int loc					= (cursor->current + 1) % hash_map->map_size;

	/* this is the current position of the cursor */
	/* and start scanning 1 ahead */

	/* start at the current position, scan forward */
	while (loc != cursor->first) {
		/* check to see if current item is a match based on key */
		/* locate first item */
		ion_hash_bucket_t *item = (((ion_hash_bucket_t *) ((hash_map->entry + (hash_map->super.record.key_size + hash_map->super.record.value_size + SIZEOF(STATUS)) * loc))));

		if ((item->status == ION_EMPTY) || (item->status == ION_DELETED)) {
			/* if empty, just skip to next cell */
			loc++;
		}
		else {
			/* check to see if the current key value satisfies the predicate */

			ion_boolean_t key_satisfies_predicate = test_predicate(&(cursor->super), item->data);	/* assumes that the key is first */

			if (key_satisfies_predicate == boolean_true) {
				cursor->current = loc;	/* this is the next index for value */
				return cs_cursor_active;
			}

			/* If valid bucket is not found, advance current position. */
			loc++;
		}

		if (loc >= hash_map->map_size) {
			/* Perform wrapping */
			loc = 0;
		}
	}

	/* if you end up here, you've wrapped the entire data structure and not found a value */
	return cs_end_of_results;
}

/**
@brief	  Finds multiple instances of a keys that satisfy the provided
			 predicate in the dictionary.

@details	Generates a cursor that allows the traversal of items where
			the items key satisfies the @p predicate (if the underlying
			implementation allows it).

@param	  dictionary
				The instance of the dictionary to search.
@param	  predicate
				The predicate to be used as the condition for matching.
@param	  cursor
				The pointer to a cursor which is caller declared but callee
				is responsible for populating.
@return	 The status of the operation.
*/
ion_err_t
oadict_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
) {
	/* allocate memory for cursor */
	if ((*cursor = malloc(sizeof(ion_oadict_cursor_t))) == NULL) {
		return err_out_of_memory;
	}

	(*cursor)->dictionary			= dictionary;
	(*cursor)->status				= cs_cursor_uninitialized;

	/* bind destroy method for cursor */
	(*cursor)->destroy				= oadict_destroy_cursor;

	/* bind correct next function */
	(*cursor)->next					= oadict_next;	/* this will use the correct value */

	/* allocate predicate */
	(*cursor)->predicate			= malloc(sizeof(ion_predicate_t));
	(*cursor)->predicate->type		= predicate->type;
	(*cursor)->predicate->destroy	= predicate->destroy;

	/* based on the type of predicate that is being used, need to create the correct cursor */
	switch (predicate->type) {
		case predicate_equality: {
			/* as this is an equality, need to malloc for key as well */
			if (((*cursor)->predicate->statement.equality.equality_value = malloc((((ion_hashmap_t *) dictionary->instance)->super.record.key_size))) == NULL) {
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.equality.equality_value, predicate->statement.equality.equality_value, ((((ion_hashmap_t *) dictionary->instance)->super.record.key_size)));

			/* find the location of the first element as this is a straight equality */
			int location = cs_invalid_index;

			if (oah_find_item_loc((ion_hashmap_t *) dictionary->instance, (*cursor)->predicate->statement.equality.equality_value, &location) == err_item_not_found) {
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else {
				(*cursor)->status = cs_cursor_initialized;

				/* cast to specific instance type for conveniences of setup */
				ion_oadict_cursor_t *oadict_cursor = (ion_oadict_cursor_t *) (*cursor);

				/* the cursor is ready to be consumed */
				oadict_cursor->first	= location;

				oadict_cursor->current	= location;
				return err_ok;
			}

			break;
		}

		case predicate_range: {
			if (((*cursor)->predicate->statement.range.lower_bound = malloc((((ion_hashmap_t *) dictionary->instance)->super.record.key_size))) == NULL) {
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.range.lower_bound, predicate->statement.range.lower_bound, (((ion_hashmap_t *) dictionary->instance)->super.record.key_size));

			/* as this is a range, need to malloc upper bound key */
			if (((*cursor)->predicate->statement.range.upper_bound = malloc((((ion_hashmap_t *) dictionary->instance)->super.record.key_size))) == NULL) {
				free((*cursor)->predicate->statement.range.lower_bound);
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.range.upper_bound, predicate->statement.range.upper_bound, (((ion_hashmap_t *) dictionary->instance)->super.record.key_size));

			ion_oadict_cursor_t *oadict_cursor	= (ion_oadict_cursor_t *) (*cursor);
			ion_hashmap_t		*hash_map		= ((ion_hashmap_t *) dictionary->instance);

			(*cursor)->status		= cs_cursor_initialized;
			oadict_cursor->first	= (hash_map->map_size) - 1;
			oadict_cursor->current	= -1;

			ion_err_t err = oadict_scan(oadict_cursor);

			if (cs_end_of_results == err) {
				(*cursor)->status = cs_cursor_uninitialized;
			}

			return err_ok;
			break;
		}

		case predicate_all_records: {
			ion_oadict_cursor_t *oadict_cursor	= (ion_oadict_cursor_t *) (*cursor);
			ion_hashmap_t		*hash_map		= ((ion_hashmap_t *) dictionary->instance);

			(*cursor)->status		= cs_cursor_initialized;
			oadict_cursor->first	= (hash_map->map_size) - 1;
			oadict_cursor->current	= -1;

			ion_err_t err = oadict_scan(oadict_cursor);

			if (cs_end_of_results == err) {
				(*cursor)->status = cs_cursor_uninitialized;
			}

			return err_ok;
			break;
		}

		case predicate_predicate: {
			break;
		}

		default: {
			return err_invalid_predicate;	/* * Invalid predicate supplied */
			break;
		}
	}

	return err_ok;
}

/**
@brief			Opens a specific open address hash instance of a dictionary.

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
oadict_open_dictionary(
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

/**
@brief			Closes an open address hash instance of a dictionary.

@param			dictionary
					A pointer to the specific dictionary instance to be closed.

@return			The status of closing the dictionary.
 */
ion_err_t
oadict_close_dictionary(
	ion_dictionary_t *dictionary
) {
	UNUSED(dictionary);
	return err_not_implemented;
}

void
oadict_init(
	ion_dictionary_handler_t *handler
) {
	handler->insert				= oadict_insert;
	handler->create_dictionary	= oadict_create_dictionary;
	handler->get				= oadict_get;
	handler->update				= oadict_update;
	handler->find				= oadict_find;
	handler->remove				= oadict_delete;
	handler->delete_dictionary	= oadict_delete_dictionary;
	handler->close_dictionary	= oadict_close_dictionary;
	handler->open_dictionary	= oadict_open_dictionary;
}

ion_status_t
oadict_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return oah_insert((ion_hashmap_t *) dictionary->instance, key, value);
}

ion_err_t
oadict_create_dictionary(
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
	/* this is the instance of the hashmap */
	dictionary->instance			= malloc(sizeof(ion_hashmap_t));

	dictionary->instance->compare	= compare;

	/* this registers the dictionary the dictionary */
	oah_initialize((ion_hashmap_t *) dictionary->instance, oah_compute_simple_hash, key_type, key_size, value_size, dictionary_size);	/* just pick an arbitary size for testing atm */

	/*TODO The correct comparison operator needs to be bound at run time
	 * based on the type of key defined
	*/

	if (NULL == handler) {
		return err_uninitialized;
	}

	/* register the correct handler */
	dictionary->handler = handler;

	return 0;
}

ion_status_t
oadict_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
) {
	return oah_delete((ion_hashmap_t *) dictionary->instance, key);
}

ion_err_t
oadict_delete_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_err_t result = oah_destroy((ion_hashmap_t *) dictionary->instance);

	free(dictionary->instance);
	dictionary->instance = NULL;/* When releasing memory, set pointer to NULL */
	return result;
}

ion_status_t
oadict_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return oah_update((ion_hashmap_t *) dictionary->instance, key, value);
}

ion_cursor_status_t
oadict_next(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
) {
	ion_oadict_cursor_t *oadict_cursor = (ion_oadict_cursor_t *) cursor;

	/* check the status of the cursor and if it is not valid or at the end, just exit */
	if (cursor->status == cs_cursor_uninitialized) {
		return cursor->status;
	}
	else if (cursor->status == cs_end_of_results) {
		return cursor->status;
	}
	else if ((cursor->status == cs_cursor_initialized) || (cursor->status == cs_cursor_active)) {
		/* cursor is active and results have never been accessed */
		/* extract reference to map */
		ion_hashmap_t *hash_map = ((ion_hashmap_t *) cursor->dictionary->instance);

		/* assume that the value has been pre-allocated */
		/* compute length of data record stored in map */
		int data_length = hash_map->super.record.key_size + hash_map->super.record.value_size;

		if (cursor->status == cs_cursor_active) {
			/* find the next valid entry */

			/* scan and determine what to do? */
			if (cs_end_of_results == oadict_scan(oadict_cursor)) {
				/* Then this is the end and there are no more results */
				cursor->status = cs_end_of_results;
				return cursor->status;
			}
		}
		else {
			/* if the cursor is initialized but not active, then just read the data and set cursor active */
			cursor->status = cs_cursor_active;
		}

		/* the results are now ready //reference item at given position */
		ion_hash_bucket_t *item = (((ion_hash_bucket_t *) ((hash_map->entry + (data_length + SIZEOF(STATUS)) * oadict_cursor->current /*idx*/))));

		memcpy(record->key, (item->data), hash_map->super.record.key_size);

		memcpy(record->value, (item->data + hash_map->super.record.key_size), hash_map->super.record.value_size);

		/* and update current cursor position */
		return cursor->status;
	}

	/* and if you get this far, the cursor is invalid */
	return cs_invalid_cursor;
}

ion_boolean_t
oadict_is_equal(
	ion_dictionary_t	*dict,
	ion_key_t			key1,
	ion_key_t			key2
) {
	if (memcmp(key1, key2, (((ion_hashmap_t *) dict->instance)->super.record.key_size)) == ION_IS_EQUAL) {
		return boolean_true;
	}
	else {
		return boolean_false;
	}
}

void
oadict_destroy_cursor(
	ion_dict_cursor_t **cursor
) {
	(*cursor)->predicate->destroy(&(*cursor)->predicate);
	free(*cursor);
	*cursor = NULL;
}
