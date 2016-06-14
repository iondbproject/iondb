/**
 *
 *
 */

#include "flat_file_dictionary_handler.h"

void
ffdict_init(
	dictionary_handler_t *handler
) {
	handler->insert				= ffdict_insert;
	handler->create_dictionary	= ffdict_create_dictionary;
	handler->get				= ffdict_query;
	handler->update				= ffdict_update;
	handler->find				= ffdict_find;
	handler->remove				= ffdict_delete;
	handler->delete_dictionary	= ffdict_delete_dictionary;
}

err_t
ffdict_insert(
	dictionary_t	*dictionary,
	ion_key_t		key,
	ion_value_t		value
) {
	return ff_insert((ff_file_t *) dictionary->instance, key, value);
}

/** @todo the value needs to be fixed */
err_t
ffdict_query(
	dictionary_t	*dictionary,
	ion_key_t		key,
	ion_value_t		value
) {
	return ff_query((ff_file_t *) dictionary->instance, key, value);
}

err_t
ffdict_create_dictionary(
	ion_dictionary_id_t			id,
	key_type_t					key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	int							dictionary_size,
	ion_dictionary_compare_t	compare,
	dictionary_handler_t		*handler,
	dictionary_t				*dictionary
) {
	dictionary->instance			= (dictionary_parent_t *) malloc(sizeof(ff_file_t));

	dictionary->instance->compare	= compare;

	/* this registers the dictionary the dictionary */
	ff_initialize((ff_file_t *) (dictionary->instance), key_type, key_size, value_size);
	/**@TODO The correct comparison operator needs to be bound at run time
	 * based on the type of key defined
	 */

	/* register the correct handler */
	dictionary->handler = handler;	/* todo: need to check to make sure that the handler is registered */

	return 0;
}

/** @todo correct return type */
err_t
ffdict_delete(
	dictionary_t	*dictionary,
	ion_key_t		key
) {
	ion_status_t status = ff_delete((ff_file_t *) dictionary->instance, key);

	return status.err;
}

err_t
ffdict_delete_dictionary(
	dictionary_t *dictionary
) {
	err_t result = ff_destroy((ff_file_t *) (dictionary->instance));

	free(dictionary->instance);
	dictionary->instance = NULL;/* When releasing memory, set pointer to NULL */
	return result;
}

err_t
ffdict_update(
	dictionary_t	*dictionary,
	ion_key_t		key,
	ion_value_t		value
) {
	return ff_update((ff_file_t *) dictionary->instance, key, value);
}

/** @todo What do we do if the cursor is already active? */
err_t
ffdict_find(
	dictionary_t	*dictionary,
	predicate_t		*predicate,
	dict_cursor_t	**cursor
) {
	/* allocate memory for cursor */
	if ((*cursor = (dict_cursor_t *) malloc(sizeof(ffdict_cursor_t))) == NULL) {
		return err_out_of_memory;
	}

	(*cursor)->dictionary			= dictionary;
	(*cursor)->status				= cs_cursor_uninitialized;

	/* bind destroy method for cursor */
	(*cursor)->destroy				= ffdict_destroy_cursor;

	/* bind correct next function */
	(*cursor)->next					= ffdict_next;	/* this will use the correct value */

	/* allocate predicate */
	(*cursor)->predicate			= (predicate_t *) malloc(sizeof(predicate_t));
	(*cursor)->predicate->type		= predicate->type;
	(*cursor)->predicate->destroy	= predicate->destroy;

	/* based on the type of predicate that is being used, need to create the correct cursor */
	switch (predicate->type) {
		case predicate_equality: {
			/* as this is an equality, need to malloc for key as well */
			if (((*cursor)->predicate->statement.equality.equality_value = (ion_key_t) malloc((int) (dictionary->instance->record.key_size))) == NULL) {
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.equality.equality_value, predicate->statement.equality.equality_value, (int) (dictionary->instance->record.key_size));

			/* find the location of the first element as this is a straight equality */
			ion_fpos_t location = cs_invalid_index;

			if (ff_find_item_loc((ff_file_t *) dictionary->instance, (*cursor)->predicate->statement.equality.equality_value, &location) == err_item_not_found) {
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else {
				(*cursor)->status = cs_cursor_initialized;

				/* cast to specific instance type for conveniences of setup */
				ffdict_cursor_t *ffdict_cursor = (ffdict_cursor_t *) (*cursor);

				/* the cursor is ready to be consumed */
				/* ffdict_cursor->first = location; */
				ffdict_cursor->current = location;
				return err_ok;
			}

			break;
		}

		case predicate_range: {
			/* as this is a range, need to malloc lower bound key */
			if (((*cursor)->predicate->statement.range.lower_bound = (ion_key_t) malloc((((ff_file_t *) dictionary->instance)->super.record.key_size))) == NULL) {
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.range.lower_bound, predicate->statement.range.lower_bound, (int) (dictionary->instance->record.key_size));

			/* as this is a range, need to malloc upper bound key */
			if (((*cursor)->predicate->statement.range.upper_bound = malloc((int) (dictionary->instance->record.key_size))) == NULL) {
				free((*cursor)->predicate->statement.range.lower_bound);
				free((*cursor)->predicate);
				free(*cursor);	/* cleanup */
				return err_out_of_memory;
			}

			/* copy across the key value as the predicate may be destroyed */
			memcpy((*cursor)->predicate->statement.range.upper_bound, predicate->statement.range.upper_bound, (int) (dictionary->instance->record.key_size));

			ffdict_cursor_t *ffdict_cursor = (ffdict_cursor_t *) (*cursor);

			/* set the start of the scan to be at the first record */
			ffdict_cursor->current = ((ff_file_t *) dictionary->instance)->start_of_data;

			/* scan for the first instance that satisfies the predicate */
			if (cs_end_of_results == ffdict_scan(ffdict_cursor)) {
				/* this will still have to be returned? */
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
ffdict_next(
	dict_cursor_t	*cursor,
	ion_record_t	*record
) {
	/* @todo if the collection changes, then the status of the cursor needs to change */
	ffdict_cursor_t *ffdict_cursor = (ffdict_cursor_t *) cursor;

	/* check the status of the cursor and if it is not valid or at the end, just exit */
	if (cursor->status == cs_cursor_uninitialized) {
		return cursor->status;
	}
	else if (cursor->status == cs_end_of_results) {
		return cursor->status;
	}
	else if ((cursor->status == cs_cursor_initialized) || (cursor->status == cs_cursor_active)) {
		/* cursor is active and results have never been accessed */
		ff_file_t *file = (ff_file_t *) (cursor->dictionary->instance);

		if (cursor->status == cs_cursor_active) {
			/* find the next valid entry */

			/* scan and determine what to do? */
			if (cs_end_of_results == ffdict_scan(ffdict_cursor)) {
				/* Then this is the end and there are no more results */
				cursor->status = cs_end_of_results;
				/** @todo need to do something with cursor? - done? */
				return cursor->status;
			}
		}
		else {
			/* if the cursor is initialized but not active, then just read the data and set cursor active */
			cursor->status = cs_cursor_active;
		}

		/* the results are now ready */
		/* reference item at given position */

		/* set the position so that it is pointing to the start of the record */
		fseek(file->file_ptr, ffdict_cursor->current + SIZEOF(STATUS), SEEK_SET);

		/* and read the key */
		fread(record->key, file->super.record.key_size, 1, file->file_ptr);

		/* and read the key */
		fread(record->value, file->super.record.value_size, 1, file->file_ptr);

		/* and update current cursor position */
		return cursor->status;
	}

	/* and if you get this far, the cursor is invalid */
	return cs_invalid_cursor;
}

void
ffdict_destroy_cursor(
	dict_cursor_t **cursor
) {
	(*cursor)->predicate->destroy(&(*cursor)->predicate);
	free(*cursor);
	*cursor = NULL;
}

boolean_t
ffdict_test_predicate(
	dict_cursor_t	*cursor,
	ion_key_t		key
) {
	/* TODO need to check key match; what's the most efficient way? */
	int key_satisfies_predicate;

	ff_file_t *file = (ff_file_t *) (cursor->dictionary->instance);

	/* pre-prime value for faster exit */
	key_satisfies_predicate = boolean_false;

	switch (cursor->predicate->type) {
		case predicate_equality:/* equality scan check */
		{
			if (IS_EQUAL == file->super.compare(cursor->predicate->statement.equality.equality_value, key, file->super.record.key_size)) {
				key_satisfies_predicate = boolean_true;
			}

			break;
		}

		case predicate_range:	/* range check */
		{
			if (/* lower_bound <= key <==> !(lower_bound > key) */
				(!(A_gt_B == file->super.compare(cursor->predicate->statement.range.lower_bound, key, file->super.record.key_size))) &&	/* key <= upper_bound <==> !(key > upper_bound) */
				(!(A_gt_B == file->super.compare(key, cursor->predicate->statement.range.upper_bound, file->super.record.key_size)))) {
				key_satisfies_predicate = boolean_true;
			}

			break;
		}
	}

	return key_satisfies_predicate;
}

err_t
ffdict_scan(
	ffdict_cursor_t *cursor	/* know exactly what implementation of cursor is */
) {
	/* need to scan hashmap fully looking for values that satisfy - need to think about */
	ff_file_t *file = (ff_file_t *) (cursor->super.dictionary->instance);

	f_file_record_t *record;

	ion_fpos_t cur_pos	= ftell(file->file_ptr);			/* this is where you are current */
	/* advance to the next record and check */

	int record_size		= SIZEOF(STATUS) + file->super.record.key_size + file->super.record.value_size;

	/* advance to the next record */
	long offset			= (cursor->current + record_size) - cur_pos;	/* compute offset */

	/* if there a move required, do it */
	if (offset != 0) {
		fseek(file->file_ptr, offset, SEEK_CUR);/* set the cursor to the correct position in file */
	}

	record = (f_file_record_t *) malloc(record_size);

	/* continue until end of file, which will error out on return from sd_fread */
	/** @todo option to increase buffer size for performance increase on IO */
	while (1 == fread(record, record_size, 1, file->file_ptr)) {
		if (record->status != DELETED) {
			/**
			 * Compares value == key
			 */
			boolean_t key_satisfies_predicate = ffdict_test_predicate(&(cursor->super), (ion_key_t) record->data);	/* assumes that the key is first */

			if (key_satisfies_predicate == boolean_true) {
				/** @TODO revisit to cache result? */
				/* back up current cursor to point at the record */
				cursor->current = ftell(file->file_ptr) - record_size;
				free(record);
				return cs_valid_data;
			}
		}

		/* otherwise scan to next record */
	}

	/* if you end up here, you've wrapped the entire data structure and not found a value */
	free(record);
	return cs_end_of_results;
}
