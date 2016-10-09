/******************************************************************************/
/**
 @file
 @author		Scott Ronald Fazackerley
 @brief		The handler for a hash table using linear probing.
*/
/******************************************************************************/

#include "open_address_file_hash_dictionary_handler.h"
#include "../../key_value/kv_system.h"
#include "open_address_file_hash.h"
#include "../../file/ion_file.h"

void
oafdict_init(
	ion_dictionary_handler_t *handler
) {
	handler->insert				= oafdict_insert;
	handler->create_dictionary	= oafdict_create_dictionary;
	handler->get				= oafdict_query;
	handler->update				= oafdict_update;
	handler->find				= oafdict_find;
	handler->remove				= oafdict_delete;
	handler->delete_dictionary	= oafdict_delete_dictionary;
	handler->open_dictionary	= oafdict_open_dictionary;
	handler->close_dictionary	= oafdict_close_dictionary;
}

ion_status_t
oafdict_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return oafh_insert((ion_file_hashmap_t *) dictionary->instance, key, value);
}

ion_status_t
oafdict_query(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return oafh_query((ion_file_hashmap_t *) dictionary->instance, key, value);
}

ion_err_t
oafdict_create_dictionary(
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
	dictionary->instance			= malloc(sizeof(ion_file_hashmap_t));

	dictionary->instance->compare	= compare;

	/* this registers the dictionary the dictionary */
	oafh_initialize((ion_file_hashmap_t *) dictionary->instance, oafh_compute_simple_hash, key_type, key_size, value_size, dictionary_size, id);/* just pick an arbitary size for testing atm */

	/*TODO The correct comparison operator needs to be bound at run time
	 * based on the type of key defined
	*/

	/* register the correct handler */
	dictionary->handler = handler;	/* todo: need to check to make sure that the handler is registered */

	return 0;
}

ion_status_t
oafdict_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
) {
	return oafh_delete((ion_file_hashmap_t *) dictionary->instance, key);
}

ion_err_t
oafdict_delete_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_err_t result = oafh_destroy((ion_file_hashmap_t *) dictionary->instance);

	free(dictionary->instance);
	dictionary->instance = NULL;/* When releasing memory, set pointer to NULL */
	return result;
}

ion_status_t
oafdict_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return oafh_update((ion_file_hashmap_t *) dictionary->instance, key, value);
}

/*@todo What do we do if the cursor is already active? */
ion_err_t
oafdict_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
) {
	/* allocate memory for cursor */
	if ((*cursor = malloc(sizeof(ion_oafdict_cursor_t))) == NULL) {
		return err_out_of_memory;
	}

	(*cursor)->dictionary			= dictionary;
	(*cursor)->status				= cs_cursor_uninitialized;

	/* bind destroy method for cursor */
	(*cursor)->destroy				= oafdict_destroy_cursor;

	/* bind correct next function */
	(*cursor)->next					= oafdict_next;	/* this will use the correct value */

	/* allocate predicate */
	(*cursor)->predicate			= malloc(sizeof(ion_predicate_t));
	(*cursor)->predicate->type		= predicate->type;
	(*cursor)->predicate->destroy	= predicate->destroy;

	/* based on the type of predicate that is being used, need to create the correct cursor */
	switch (predicate->type) {
		case predicate_equality: {
			/* as this is an equality, need to malloc for key as well */
			if (((*cursor)->predicate->statement.equality.equality_value = malloc((((ion_file_hashmap_t *) dictionary->instance)->super.record.key_size))) == NULL) {
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.equality.equality_value, predicate->statement.equality.equality_value, ((((ion_file_hashmap_t *) dictionary->instance)->super.record.key_size)));

			/* find the location of the first element as this is a straight equality */
			int location = cs_invalid_index;

			if (oafh_find_item_loc((ion_file_hashmap_t *) dictionary->instance, (*cursor)->predicate->statement.equality.equality_value, &location) == err_item_not_found) {
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else {
				(*cursor)->status = cs_cursor_initialized;

				/* cast to specific instance type for conveniences of setup */
				ion_oafdict_cursor_t *oadict_cursor = (ion_oafdict_cursor_t *) (*cursor);

				/* the cursor is ready to be consumed */
				oadict_cursor->first	= location;

				oadict_cursor->current	= location;
				return err_ok;
			}

			break;
		}

		case predicate_range: {
			/* as this is a range, need to malloc lower bound key */
			if (((*cursor)->predicate->statement.range.lower_bound = malloc((((ion_file_hashmap_t *) dictionary->instance)->super.record.key_size))) == NULL) {
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.range.lower_bound, predicate->statement.range.lower_bound, (((ion_file_hashmap_t *) dictionary->instance)->super.record.key_size));

			/* as this is a range, need to malloc upper bound key */
			if (((*cursor)->predicate->statement.range.upper_bound = malloc((((ion_file_hashmap_t *) dictionary->instance)->super.record.key_size))) == NULL) {
				free((*cursor)->predicate->statement.range.lower_bound);
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.range.upper_bound, predicate->statement.range.upper_bound, (((ion_file_hashmap_t *) dictionary->instance)->super.record.key_size));
		}

		/* Range query will intentionally continue to all record code to get rid of duplicate statements. */
		case predicate_all_records: {
			ion_oafdict_cursor_t	*oafdict_cursor = (ion_oafdict_cursor_t *) (*cursor);
			ion_file_hashmap_t		*hash_map		= ((ion_file_hashmap_t *) dictionary->instance);

			(*cursor)->status		= cs_cursor_initialized;
			oafdict_cursor->first	= (hash_map->map_size) - 1;
			oafdict_cursor->current = -1;

			ion_err_t err = oafdict_scan(oafdict_cursor);

			if (cs_valid_data != err) {
				(*cursor)->status = cs_end_of_results;
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

ion_cursor_status_t
oafdict_next(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
) {
	/* @todo if the dictionary instance changes, then the status of the cursor needs to change */
	ion_oafdict_cursor_t *oafdict_cursor = (ion_oafdict_cursor_t *) cursor;

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
		ion_file_hashmap_t *hash_map = ((ion_file_hashmap_t *) cursor->dictionary->instance);

		/* assume that the value has been pre-allocated */
		/* compute length of data record stored in map */
		int data_length = hash_map->super.record.key_size + hash_map->super.record.value_size;

		if (cursor->status == cs_cursor_active) {
			/* find the next valid entry */

			/* scan and determine what to do? */
			if (cs_end_of_results == oafdict_scan(oafdict_cursor)) {
				/* todo - need to read and updat file position */
				/* Then this is the end and there are no more results */
				cursor->status = cs_end_of_results;
				/*@todo need to do something with cursor? - done? */
				return cursor->status;
			}
		}
		else {
			/* if the cursor is initialized but not active, then just read the data and set cursor active */
			cursor->status = cs_cursor_active;
		}

		/* the results are now ready //reference item at given position */

		/* set position in file to read value */
		fseek(hash_map->file, (ION_SIZEOF(STATUS) + data_length) * oafdict_cursor->current	/* position is based on indexes (not abs file pos) */
			+ ION_SIZEOF(STATUS), SEEK_SET);

/*@todo this needs to be addressed in terms of return type
*/
		fread(record->key, hash_map->super.record.key_size, 1, hash_map->file);
		fread(record->value, hash_map->super.record.value_size, 1, hash_map->file);

		/* and update current cursor position */
		return cursor->status;
	}

	/* and if you get this far, the cursor is invalid */
	return cs_invalid_cursor;
}

ion_boolean_t
oafdict_is_equal(
	ion_dictionary_t	*dict,
	ion_key_t			key1,
	ion_key_t			key2
) {
	if (memcmp(key1, key2, (((ion_file_hashmap_t *) dict->instance)->super.record.key_size)) == IS_EQUAL) {
		return boolean_true;
	}
	else {
		return boolean_false;
	}
}

void
oafdict_destroy_cursor(
	ion_dict_cursor_t **cursor
) {
	(*cursor)->predicate->destroy(&(*cursor)->predicate);
	free(*cursor);
	*cursor = NULL;
}

ion_boolean_t
oafdict_test_predicate(
	ion_dict_cursor_t	*cursor,
	ion_key_t			key
) {
	/* TODO need to check key match; what's the most efficient way? */

	/**
	 * Compares value == key
	*/
	int					key_satisfies_predicate;
	ion_file_hashmap_t	*hash_map = (ion_file_hashmap_t *) (cursor->dictionary->instance);

	/* pre-prime value for faster exit */
	key_satisfies_predicate = boolean_false;

	switch (cursor->predicate->type) {
		case predicate_equality:/* equality scan check */
		{
			if (IS_EQUAL == hash_map->super.compare(cursor->predicate->statement.equality.equality_value, key, hash_map->super.record.key_size)) {
				key_satisfies_predicate = boolean_true;
			}

			break;
		}

		case predicate_range:	/* range check */
		{
			if (/* lower_bound <= key <==> !(lower_bound > key) */
				(!(A_gt_B == hash_map->super.compare(cursor->predicate->statement.range.lower_bound, key, hash_map->super.record.key_size))) &&	/* key <= upper_bound <==> !(key > upper_bound) */
				(!(A_gt_B == hash_map->super.compare(key, cursor->predicate->statement.range.upper_bound, hash_map->super.record.key_size)))) {
				key_satisfies_predicate = boolean_true;
			}

			break;
		}

		case predicate_all_records: {
			key_satisfies_predicate = boolean_true;
			break;
		}
	}

	return key_satisfies_predicate;
}

ion_err_t
oafdict_scan(
	ion_oafdict_cursor_t *cursor/* know exactly what implementation of cursor is */
) {
	/* need to scan hashmap fully looking for values that satisfy - need to think about */
	ion_file_hashmap_t *hash_map	= (ion_file_hashmap_t *) (cursor->super.dictionary->instance);

	int loc							= (cursor->current + 1) % hash_map->map_size;
	/* this is the current position of the cursor */
	/* and start scanning 1 ahead */

	int record_size = ION_SIZEOF(STATUS) + hash_map->super.record.key_size + hash_map->super.record.value_size;

	/* move to the correct position in the fie */
	fseek(hash_map->file, loc * record_size, SEEK_SET);

	ion_hash_bucket_t *item;

	item = malloc(record_size);

	/* start at the current position, scan forward */
	while (loc != cursor->first) {
		fread(item, record_size, 1, hash_map->file);

		if ((item->status == ION_EMPTY) || (item->status == DELETED)) {
			/* if empty, just skip to next cell */
			loc++;
		}
		else {
			/* check to see if the current key value satisfies the predicate */

			/* TODO need to check key match; what's the most efficient way? */

			ion_boolean_t key_satisfies_predicate = oafdict_test_predicate(&(cursor->super), item->data);	/* assumes that the key is first */

			if (key_satisfies_predicate == boolean_true) {
				cursor->current = loc;	/* this is the next index for value */
				free(item);
				return cs_valid_data;
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
	free(item);
	return cs_end_of_results;
}

ion_err_t
oafdict_open_dictionary(
	ion_dictionary_handler_t		*handler,
	ion_dictionary_t				*dictionary,
	ion_dictionary_config_info_t	*config,
	ion_dictionary_compare_t		compare
) {
	return oafdict_create_dictionary(config->id, config->type, config->key_size, config->value_size, config->dictionary_size, compare, handler, dictionary);
}

ion_err_t
oafdict_close_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_file_hashmap_t	*hash_map;
	ion_err_t			err;

	hash_map	= (ion_file_hashmap_t *) dictionary->instance;
	err			= oafh_close(hash_map);

	/* The following line creates an allocation error. Will not including it create a memory leak? */
/*	free(dictionary->instance); */

	dictionary->instance = NULL;

	if (err_ok != err) {
		return err_dictionary_destruction_error;
	}

	return err_ok;
}
