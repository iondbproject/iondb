/******************************************************************************/
/**
 @file	  linear_hash_dictionary_handler.c
 @author		Scott Ronald Fazackerley
 @brief		The handler for a hash table using linear probing.
 */
/******************************************************************************/

#include "linear_hash_dictionary_handler.h"
#include "./../../kv_system.h"

void
lhdict_init(
	dictionary_handler_t *handler
) {
	handler->insert				= lhdict_insert;
	handler->create_dictionary	= lhdict_create_dictionary;
	handler->get				= lhdict_query;
	handler->update				= lhdict_update;
	handler->find				= lhdict_find;
	handler->remove				= lhdict_delete;
	handler->delete_dictionary	= lhdict_delete_dictionary;
	handler->open_dictionary	= NULL;
	handler->close_dictionary	= NULL;
}

err_t
lhdict_open(
	dictionary_handler_t			*handler,
	dictionary_t					*dict,
	ion_dictionary_config_info_t	*config
) {
	UNUSED(handler);
	UNUSED(dict);
	UNUSED(config);
	return err_not_implemented;
}

err_t
lhdict_close(
	dictionary_t *dict
) {
	UNUSED(dict);
	return err_not_implemented;
}

ion_status_t
lhdict_insert(
	dictionary_t	*dictionary,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_status_t status = lh_insert((linear_hashmap_t *) dictionary->instance, key, value);

	/*Check load factor and split if necessary*/

	if (((linear_hashmap_t *) dictionary->instance)->use_split) {
		if (SPLIT_THRESHOLD < lh_compute_load_factor((linear_hashmap_t *) dictionary->instance)) {
			lh_split((linear_hashmap_t *) dictionary->instance);
		}
	}

	return status;
}

ion_status_t
lhdict_query(
	dictionary_t	*dictionary,
	ion_key_t		key,
	ion_value_t		value
) {
	return lh_query((linear_hashmap_t *) dictionary->instance, key, value);
}

err_t
lhdict_create_dictionary(
	ion_dictionary_id_t			id,
	key_type_t					key_type,
	int							key_size,
	int							value_size,
	int							dictionary_size,
	ion_dictionary_compare_t	compare,
	dictionary_handler_t		*handler,
	dictionary_t				*dictionary
) {
	/* this is the instance of the hashmap */

	dictionary->instance			= (dictionary_parent_t *) malloc(sizeof(linear_hashmap_t));

	dictionary->instance->compare	= compare;

	/* this registers the dictionary the dictionary */
	err_t err = lh_initialize((linear_hashmap_t *) dictionary->instance, lh_compute_hash, key_type, key_size, value_size, dictionary_size, id);

	/*TODO The correct comparison operator needs to be bound at run time
	 * based on the type of key defined
	 */

	if (err == err_ok) {
		/* register the correct handler */
		dictionary->handler = handler;	/* todo: need to check to make sure that the handler is registered */
		return err_ok;
	}
	else {
		/*free instance as dictionary cannot be created */
		free(dictionary->instance);
		return err;
	}
}

ion_status_t
lhdict_delete(
	dictionary_t	*dictionary,
	ion_key_t		key
) {
	return lh_delete((linear_hashmap_t *) dictionary->instance, key);
}

err_t
lhdict_delete_dictionary(
	dictionary_t *dictionary
) {
	err_t result = lh_destroy((linear_hashmap_t *) dictionary->instance);

	free(dictionary->instance);
	dictionary->instance = NULL;/* When releasing memory, set pointer to NULL */
	return result;
}

ion_status_t
lhdict_update(
	dictionary_t	*dictionary,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_status_t status = lh_update((linear_hashmap_t *) dictionary->instance, key, value);

	return status;
}

/*@todo What do we do if the cursor is already active? */
err_t
lhdict_find(
	dictionary_t	*dictionary,
	predicate_t		*predicate,
	dict_cursor_t	**cursor
) {
	/* allocate memory for cursor */
	if ((*cursor = (dict_cursor_t *) malloc(sizeof(lhdict_cursor_t))) == NULL) {
		return err_out_of_memory;
	}

	(*cursor)->dictionary								= dictionary;
	(*cursor)->status									= cs_cursor_uninitialized;

	/* bind destroy method for cursor */
	(*cursor)->destroy									= lhdict_destroy_cursor;

	/* bind correct next function */
	(*cursor)->next										= lhdict_next;	/* this will use the correct value */

	/* allocate predicate */
	(*cursor)->predicate								= (predicate_t *) malloc(sizeof(predicate_t));
	(*cursor)->predicate->type							= predicate->type;
	(*cursor)->predicate->destroy						= predicate->destroy;

	((lhdict_cursor_t *) *cursor)->current_bucket		= -1;	/* Set initial pointers to INVALID */
	((lhdict_cursor_t *) *cursor)->first_bucket			= -1;
	((lhdict_cursor_t *) *cursor)->overflow				= NULL;
	((lhdict_cursor_t *) *cursor)->record_pntr			= 0;/* Point to first item in page */
	((lhdict_cursor_t *) *cursor)->evaluate_predicate	= lhdict_test_predicate;

	/* based on the type of predicate that is being used, need to create the correct cursor */
	switch (predicate->type) {
		case predicate_equality: {
			/* as this is an equality, need to malloc for key as well */
			if (((*cursor)->predicate->statement.equality.equality_value = (ion_key_t) malloc((((linear_hashmap_t *) dictionary->instance)->super.record.key_size))) == NULL) {
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.equality.equality_value, predicate->statement.equality.equality_value, ((((linear_hashmap_t *) dictionary->instance)->super.record.key_size)));

			/*@TODO find item needs to be fixed  - But how??*/
			if (lh_find((linear_hashmap_t *) dictionary->instance, *cursor) == err_item_not_found) {
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else {
				(*cursor)->status = cs_cursor_initialized;
				/*@TODO  is there something that needs to be done here ?*/
				return err_ok;
			}

			break;
		}

		case predicate_range: {
			/* as this is a range, need to malloc lower bound key */
			if (((*cursor)->predicate->statement.range.lower_bound = (ion_key_t) malloc((((linear_hashmap_t *) dictionary->instance)->super.record.key_size))) == NULL) {
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.range.lower_bound, predicate->statement.range.lower_bound, (((linear_hashmap_t *) dictionary->instance)->super.record.key_size));

			/* as this is a range, need to malloc upper bound key */
			if (((*cursor)->predicate->statement.range.upper_bound = (ion_key_t) malloc((((linear_hashmap_t *) dictionary->instance)->super.record.key_size))) == NULL) {
				free((*cursor)->predicate->statement.range.lower_bound);
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.range.upper_bound, predicate->statement.range.upper_bound, (((linear_hashmap_t *) dictionary->instance)->super.record.key_size));

			/* start at the lowest end of the range and check */
			if (lh_find((linear_hashmap_t *) dictionary->instance, *cursor) == err_item_not_found) {
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else {
				(*cursor)->status = cs_cursor_initialized;
				return err_ok;
			}

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

cursor_status_t
lhdict_next(
	dict_cursor_t	*cursor,
	ion_record_t	*record
) {
	/*@TODO if the collection changes, then the status of the cursor needs to change */
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
		linear_hashmap_t *hash_map	= ((linear_hashmap_t *) cursor->dictionary->instance);

		lh_page_cache_t *cache		= NULL;

		/*If the cursor is not active, then you can just pull the value
		 * otherwise you need to keep traversing to see if the value is valid
		 */

		int current_size = hash_map->initial_map_size * (1 << hash_map->file_level) + hash_map->bucket_pointer;

		/*Could have to search multiple pages */
		do {
			if (((lhdict_cursor_t *) cursor)->overflow == NULL) {
				/*till searching in pp*/
				/*The primary page needs to be processed */
				cursor->status = cs_cursor_active;

				/*Cache pp and search for next value*/
				lh_cache_pp((linear_hashmap_t *) cursor->dictionary->instance, 0, ((lhdict_cursor_t *) cursor)->current_bucket, &cache);

				/*search for next value record */
				err_t err = lh_search_primary_page((linear_hashmap_t *) cursor->dictionary->instance, cache, (lhdict_cursor_t *) cursor);

				if (err == err_item_not_found) {
					/*if the item is not found, invalidate the cursor */

					if (cursor->predicate->type == predicate_equality) {
						/*in the event of a strict equality and there is an empty slot, search in done */
						cursor->status = cs_end_of_results;
						return cursor->status;
					}

					/*else you will need to keep on searching */
					/*and in fact there should not be an overflow page associated with a primary page that
					 *  has an empty slot */
				}
				else if (err == err_ok) {
					/*the value has been found */
					l_hash_bucket_t *item = NULL;

					lh_read_cache(hash_map, cache, (((lhdict_cursor_t *) cursor)->record_pntr), (void *) &item);

					memcpy(record->key, item->data, hash_map->super.record.key_size);
					memcpy(record->value, item->data + hash_map->super.record.key_size, hash_map->super.record.value_size);
					/*nd update current cursor position to point to the next position*/
					((lhdict_cursor_t *) cursor)->record_pntr++;/*what happens if we exceed page */
					return cursor->status;
				}
				else {
					/*the value is not in the pp -> move onto overflow pages */
#if DEBUG
					io_printf("scanning overflow\n");
#endif

					if (((lhdict_cursor_t *) cursor)->overflow == NULL) {
						/*no open overflow file */
						((lhdict_cursor_t *) cursor)->overflow = (ll_file_t *) malloc(sizeof(ll_file_t));

						if (fll_open(((lhdict_cursor_t *) cursor)->overflow, fll_compare, hash_map->super.record.key_size, hash_map->super.record.value_size, ((lhdict_cursor_t *) cursor)->current_bucket, hash_map->id) == err_item_not_found) {
							/*in this case, cursor is null as value has not been found */
							if (cursor->predicate->type == predicate_equality) {
								cursor->status							= cs_end_of_results;
								free(((lhdict_cursor_t *) cursor)->overflow);
								((lhdict_cursor_t *) cursor)->overflow	= NULL;
								return cursor->status;
							}
							else {
								/*eed to process next page */
								free(((lhdict_cursor_t *) cursor)->overflow);
								((lhdict_cursor_t *) cursor)->overflow = NULL;
								/*eset and advance? */
							}
						}
						else {
							/*reset ll cursor and start searching file */
							fll_reset(((lhdict_cursor_t *) cursor)->overflow);
						}
					}
				}
			}

			/*only process if overflow is active */
			if (((lhdict_cursor_t *) cursor)->overflow != NULL) {
				/*as the overflow file pntr is not null, the process */
				ll_file_node_t *item = (ll_file_node_t *) malloc(((lhdict_cursor_t *) cursor)->overflow->node_size);

				if (cs_cursor_active == cursor->status) {
					while (fll_next(((lhdict_cursor_t *) cursor)->overflow, item) != err_item_not_found) {
						/*now we are at the end of the list, but what to do if we need to go onto next bucket ?*/
						troolean_t value = ((lhdict_cursor_t *) cursor)->evaluate_predicate(cursor, item->data);

						if (IS_EQUAL == value) {
							/*a match has been found */
							memcpy(record->key, item->data, hash_map->super.record.key_size);
							memcpy(record->value, item->data + hash_map->super.record.key_size, hash_map->super.record.value_size);
							/* and update current cursor position */
							free(item);
							return cursor->status;
						}
						else if (IS_GREATER == value) {
							/*as list is ordered just leave */
							break;
						}
					}

					/*if you have reached the end of the ll with a equality, leave */
					if (cursor->predicate == predicate_equality) {
						cursor->status = cs_end_of_results;
						free(item);
						return cursor->status;
					}
				}
				else {
					/*that value is already sitting at the cursor */
					cursor->status = cs_cursor_active;
					fll_get(((lhdict_cursor_t *) cursor)->overflow, item);
					memcpy(record->key, item->data, hash_map->super.record.key_size);
					memcpy(record->value, item->data + hash_map->super.record.key_size, hash_map->super.record.value_size);
					free(item);
					return cursor->status;
				}

				/*and if you get this far, close the file*/
				free(item);
				fll_close(((lhdict_cursor_t *) cursor)->overflow);
				free(((lhdict_cursor_t *) cursor)->overflow);
				((lhdict_cursor_t *) cursor)->overflow = NULL;
			}

			/*If you have reached this point, you have exhausted the pp and overflow page for a linear hash
			 *  so move onto the next bucket */
#if DEBUG
			io_printf("Incrementing bucket pointer\n");
			DUMP(((lhdict_cursor_t *) cursor)->current_bucket, "%i");
			DUMP(current_size, "%i");
#endif
			((lhdict_cursor_t *) cursor)->current_bucket = (((lhdict_cursor_t *) cursor)->current_bucket + 1) % current_size;
#if DEBUG
			DUMP(((lhdict_cursor_t *) cursor)->current_bucket, "%i");
#endif
			/*Reset to the top of the next primary page */
			((lhdict_cursor_t *) cursor)->record_pntr = 0;
		} while (((lhdict_cursor_t *) cursor)->current_bucket != ((lhdict_cursor_t *) cursor)->first_bucket);

		/*and if you make it this far, you have traversed the entire resultset*/
		cursor->status = cs_end_of_results;
		return cursor->status;
	}

	/* and if you get this far, the cursor is invalid */
	return cs_invalid_cursor;
}

void
lhdict_destroy_cursor(
	dict_cursor_t **cursor
) {
	(*cursor)->predicate->destroy(&(*cursor)->predicate);

	/*free overflow file info */
	if (((lhdict_cursor_t *) *cursor)->overflow != NULL) {
		fll_close(((lhdict_cursor_t *) *cursor)->overflow);
		free(((lhdict_cursor_t *) *cursor)->overflow);
		((lhdict_cursor_t *) *cursor)->overflow = NULL;
	}

	/*and free cursor pointer */
	free(*cursor);
	*cursor = NULL;
}

troolean_t
lhdict_test_predicate(
	dict_cursor_t	*cursor,
	ion_key_t		key
) {
	/* need to check key match; what's the most efficient way? */
	/* need to use fnptr here for the correct matching */
	/*int key_is_eq ual = memcmp(item->data, "10{" ,
	 hash_map->record.key_size);*/
	/**
	 * Compares value == key
	 */
	troolean_t			key_satisfies_predicate;
	linear_hashmap_t	*hash_map = (linear_hashmap_t *) (cursor->dictionary->instance);

	/* pre-prime value for faster exit */
	key_satisfies_predicate = boolean_false;

	switch (cursor->predicate->type) {
		case predicate_equality:/* equality scan check */
		{
			/*if key < predicate -> -1
			 * if key == predicate -> 0
			 * if key > predicate 0 -> 1
			 */
			return hash_map->super.compare(key, cursor->predicate->statement.equality.equality_value, hash_map->super.record.key_size);
		}

		case predicate_range:	/* range check */
		{
			/*FIXME Correct so that range works to short circuit early */
#if DEBUG
			io_printf("Evaluating range\n");
			DUMP(*(int *) cursor->predicate->statement.range.upper_bound, "%i");
			DUMP(*(int *) cursor->predicate->statement.range.lower_bound, "%i");
			DUMP(*(int *) key, "%i");
#endif

			if (/* upper_bound <= key <==> !(upper_bound > key) */
				(A_lt_B == hash_map->super.compare(key, cursor->predicate->statement.range.upper_bound, hash_map->super.record.key_size))) {
				key_satisfies_predicate = IS_LESS;
			}
			else if (	/* key <= lower_bound <==> !(key > lower_bound) */
				(A_gt_B == hash_map->super.compare(key, cursor->predicate->statement.range.lower_bound, hash_map->super.record.key_size))) {
				key_satisfies_predicate = IS_GREATER;
			}
			else {
				key_satisfies_predicate = IS_EQUAL;
			}

			break;
		}
	}

	return key_satisfies_predicate;
}

err_t
lhdict_scan(
	lhdict_cursor_t *cursor	/* know exactly what implementation of cursor is */
) {
	/* need to scan hashmap fully looking for values that satisfy - need to think about */
	linear_hashmap_t *hash_map	= (linear_hashmap_t *) (cursor->super.dictionary->instance);

	int loc						= 0;/* (cursor->current + 1) % hash_map->map_size; */
	/* this is the current position of the cursor */
	/* and start scanning 1 ahead */

	int record_size = SIZEOF(STATUS) + hash_map->super.record.key_size + hash_map->super.record.value_size;

	/* move to the correct position in the fie */
	fseek(hash_map->file, loc * record_size, SEEK_SET);

	return cs_end_of_results;
}
