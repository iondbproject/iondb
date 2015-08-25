#include "ff_dictionary_handler_raw.h"

/**redefines print operations for Arduino */
#ifdef DEBUG
#include "serial_c_iface.h"
#endif

void 
ffdict_init(
	dictionary_handler_t *handler
)
{
	handler->insert 			= ffdict_insert;
	handler->create_dictionary 	= ffdict_create_dictionary;
	handler->get 				= ffdict_query;
	handler->update 			= ffdict_update;
	handler->find 				= ffdict_find;
	handler->remove 			= ffdict_delete;
	handler->delete_dictionary 	= ffdict_delete_dictionary;
}

err_t
ffdict_insert(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t 	value
)
{
	return ff_insert((ff_file_t *) dictionary->instance, key, value);
}

/** @todo the value needs to be fixed */
err_t
ffdict_query(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t 	value
)
{
	return ff_query((ff_file_t *) dictionary->instance, key, value);
}

err_t
ffdict_create_dictionary(
    key_type_t 					key_type,
    ion_key_size_t				key_size,
    ion_value_size_t			value_size,
    int 						dictionary_size,
    ion_dictionary_compare_t 	compare,
    dictionary_handler_t 		*handler,
    dictionary_t 				*dictionary
)
{
	dictionary->instance = (dictionary_parent_t *) malloc(sizeof(ff_file_t));
	dictionary->instance->compare = compare;

#ifdef USE_IONFS
	ionfs_t *file = malloc(sizeof(ionfs_t));
	ionfs_format_device(sd_raw_card_size(), 512, 64, 1, 0);
	ionfs_open(file, 1);

	((ff_file_t *) (dictionary->instance))->file_ptr = file;
#endif /* USE_IONFS */

	//this registers the dictionary the dictionary
	if (ff_initialize((ff_file_t *) (dictionary->instance), key_type, key_size, value_size))
	{
		free(dictionary->instance);
		return -1; // TODO
	}

#ifdef DEBUG
	printf("Flat file initialized\n");
#endif

	/**@TODO The correct comparison operator needs to be bound at run time
	 * based on the type of key defined
	 */

	//register the correct handler
	dictionary->handler = handler;//todo: need to check to make sure that the handler is registered

	return 0;
}

/** @todo correct return type */
err_t
ffdict_delete(
	dictionary_t 	*dictionary,
	ion_key_t 		key
)
{
	return_status_t status = ff_delete((ff_file_t *) dictionary->instance, key);
	return status.err;
}

err_t
ffdict_delete_dictionary(
	dictionary_t *dictionary
)
{
	ionfs_t *file = ((ff_file_t *) (dictionary->instance))->file_ptr;
	err_t result = ff_destroy((ff_file_t *) (dictionary->instance));

#ifdef USE_IONFS
	ionfs_close(file);
	free(file);
#endif

	free(dictionary->instance);
	dictionary->instance = NULL;// When releasing memory, set pointer to NULL

	return result;
}

err_t
ffdict_update(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t 	value
)
{
	return ff_update((ff_file_t *) dictionary->instance, key, value);
}

/** @todo What do we do if the cursor is already active? */
err_t ffdict_find(
    dictionary_t 	*dictionary,
    predicate_t 	*predicate,
    dict_cursor_t 	**cursor
)
{
	//allocate memory for cursor
	if ((*cursor = (dict_cursor_t *) malloc(sizeof(ffdict_cursor_t))) == NULL)
	{
		return err_out_of_memory;
	}
	(*cursor)->dictionary = dictionary;
	(*cursor)->status = cs_cursor_uninitialized;

	//bind destroy method for cursor
	(*cursor)->destroy = ffdict_destroy_cursor;

	//bind correct next function
	(*cursor)->next = ffdict_next;	// this will use the correct value

	//allocate predicate
	(*cursor)->predicate 			= (predicate_t *) malloc(sizeof(predicate_t));
	(*cursor)->predicate->type 		= predicate->type;
	(*cursor)->predicate->destory 	= predicate->destroy;

	//based on the type of predicate that is being used, need to create the correct cursor
	switch (predicate->type)
	{
		case predicate_equality:
		{
			//as this is an equality, need to malloc for key as well
			if (((*cursor)->predicate->statement.equality.equality_value =
			   	(ion_key_t) malloc((int) (dictionary->instance->record.key_size))) 
				 == NULL)
			{
				free((*cursor)->predicate);
				free(*cursor);						//cleanup
				return err_out_of_memory;
			}
			//copy across the key value as the predicate may be destroyed
			memcpy((*cursor)->predicate->statement.equality.equality_value,
			        predicate->statement.equality.equality_value,
			        (int) (dictionary->instance->record.key_size));

			//find the location of the first element as this is a straight equality
			uint32_t block_address = 1; // TODO: define default
			uint16_t block_current_record = 0;

			if (ff_find_item_loc((ff_file_t*) dictionary->instance,
			    (*cursor)->predicate->statement.equality.equality_value,
			     &block_address, &block_current_record) == err_item_not_found)
			{
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else
			{
				(*cursor)->status = cs_cursor_initialized;
				//cast to specific instance type for conveniences of setup
				ffdict_cursor_t *ffdict_cursor = (ffdict_cursor_t *) (*cursor);
				// the cursor is ready to be consumed
				ffdict_cursor->block_address = block_address;
				ffdict_cursor->block_current_record = block_current_record;
				return err_ok;
			}
			break;
		}
		case predicate_range:
		{
			//as this is a range, need to malloc lower bound key
			if (((*cursor)->predicate->statement.range.lower_bound =
			        (ion_key_t) malloc((((ff_file_t*) dictionary->instance)->super.record.key_size)))
			        == NULL)
			{
				free((*cursor)->predicate);
				free(*cursor);					//cleanup
				return err_out_of_memory;
			}
			//copy across the key value as the predicate may be destroyed
			memcpy((*cursor)->predicate->statement.range.lower_bound,
			        predicate->statement.range.lower_bound,
			        (int)(dictionary->instance->record.key_size));

			//as this is a range, need to malloc upper bound key
			if (((*cursor)->predicate->statement.range.upper_bound =
			        (ion_key_t) malloc((((ff_file_t*) dictionary->instance)->super.record.key_size)))
			        == NULL)
			{
				free((*cursor)->predicate->statement.range.lower_bound);
				free((*cursor)->predicate);
				free(*cursor);					//cleanup
				return err_out_of_memory;
			}
			//copy across the key value as the predicate may be destroyed
			memcpy((*cursor)->predicate->statement.range.upper_bound,
			        predicate->statement.range.upper_bound,
			        (int) (dictionary->instance->record.key_size));


			ffdict_cursor_t *ffdict_cursor = (ffdict_cursor_t *) (*cursor);
			//set the start of the scan to be at the first record
			ffdict_cursor->block_address = 1; //TODO: define start of data
			ffdict_cursor->block_current_record = 0;

			//scan for the first instance that satisfies the predicate
			if (cs_end_of_results == ffdict_scan(ffdict_cursor))
			{
				//this will still have to be returned?
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else
			{
				(*cursor)->status = cs_cursor_initialized;
				return err_ok;
			}
			break;
		}
		case predicate_predicate:
		{
			break;
		}
		default:
		{
			return err_invalid_predicate;		//* Invalid predicate supplied
			break;
		}
	}
	return err_ok;
}

cursor_status_t
ffdict_next(
	dict_cursor_t 	*cursor,
	ion_record_t 	*record
)
{
	// @todo if the collection changes, then the status of the cursor needs to change
	ffdict_cursor_t *ffdict_cursor = (ffdict_cursor_t *) cursor;

	//check the status of the cursor and if it is not valid or at the end, just exit
	if (cursor->status == cs_cursor_uninitialized)
		return cursor->status;
	else if (cursor->status == cs_end_of_results)
		return cursor->status;
	else if ((cursor->status == cs_cursor_initialized)
	        || (cursor->status == cs_cursor_active))//cursor is active and results have never been accessed
	{
		ff_file_t *file = (ff_file_t*) (cursor->dictionary->instance);

		if (cursor->status == cs_cursor_active)		//find the next valid entry
		{
			//scan and determine what to do?
			if (cs_end_of_results == ffdict_scan(ffdict_cursor))
			{
				//Then this is the end and there are no more results
				cursor->status = cs_end_of_results;
				/** @todo need to do something with cursor? - done? */
				return cursor->status;
			}
		}
		else
		{
			//if the cursor is initialized but not active, then just read the data and set cursor active
			cursor->status = cs_cursor_active;
		}

		//the results are now ready
		//reference item at given position

		int record_size = SIZEOF(STATUS)+ file->super.record.key_size + file->super.record.value_size;
		//and read the key

#ifdef USE_IONFS
		ionfs_read(file->file_ptr, ffdict_cursor->block_address, record->key, file->super.record.key_size, (ffdict_cursor->block_current_record * record_size) + SIZEOF(STATUS));
#else
		device_read(ffdict_cursor->block_address, record->key, file->super.record.key_size, (ffdict_cursor->block_current_record * record_size) + SIZEOF(STATUS));
#endif

		//and read the value
#ifdef USE_IONFS
		ionfs_read(file->file_ptr, ffdict_cursor->block_address, record->value, file->super.record.value_size, (ffdict_cursor->block_current_record * record_size) + SIZEOF(STATUS) + file->super.record.key_size);
#else
		device_read(ffdict_cursor->block_address, record->value, file->super.record.value_size, (ffdict_cursor->block_current_record * record_size) + SIZEOF(STATUS) + file->super.record.key_size);
#endif

		(ffdict_cursor->block_current_record)++;

		//and update current cursor position
		return cursor->status;
	}
	//and if you get this far, the cursor is invalid
	return cs_invalid_cursor;
}

void
ffdict_destroy_cursor(
	dict_cursor_t **cursor
)
{
	(*cursor)->predicate->destroy(&(*cursor)->predicate);
	free(*cursor);
	*cursor = NULL;
}

boolean_t
ffdict_test_predicate(
	dict_cursor_t 	*cursor,
	ion_key_t 		key
)
{
	//TODO need to check key match; what's the most efficient way?
	int key_satisfies_predicate;

	ff_file_t * file = (ff_file_t *) (cursor->dictionary->instance);

	//pre-prime value for faster exit
	key_satisfies_predicate = boolean_false;

	switch (cursor->predicate->type)
	{
		case predicate_equality: //equality scan check
		{
			if (IS_EQUAL
			        == file->super.compare(
			                cursor->predicate->statement.equality.equality_value,
			                key, file->super.record.key_size))
			{
				key_satisfies_predicate = boolean_true;
			}
			break;
		}
		case predicate_range: // range check
		{
			if (		// lower_bound <= key <==> !(lower_bound > key)
			(!(A_gt_B
			        == file->super.compare(
			                cursor->predicate->statement.range.lower_bound,
			        		key,
			                file->super.record.key_size))) &&// key <= upper_bound <==> !(key > upper_bound)
			        (!(A_gt_B
			                == file->super.compare(
			                        key,
			                        cursor->predicate->statement.range.upper_bound,
			                        file->super.record.key_size))))
			{
				key_satisfies_predicate = boolean_true;
			}
			break;
		}
	}
	return key_satisfies_predicate;
}

err_t 
ffdict_scan(
	ffdict_cursor_t *cursor  //know exactly what implementation of cursor is
)
{
	//need to scan hashmap fully looking for values that satisfy - need to think about
	ff_file_t * file = (ff_file_t *) (cursor->super.dictionary->instance);

	f_file_record_t * record;

	int record_size = SIZEOF(STATUS)
						+ file->super.record.key_size
						+ file->super.record.value_size;

	if ((record = (f_file_record_t *) malloc(record_size)) == NULL)
	{
		return err_out_of_memory;
	}

#ifdef USE_SD_RAW
	if (sd_raw_read_continuous_start(cursor->block_address))
	{
		free(record);
		return err_file_incomplete_read;
	}
#endif /* USE_SD_RAW */

	do
	{
		if ((record_size * ((cursor->block_current_record) + 1)) >= 512)
		{
			(cursor->block_address)++;
			cursor->block_current_record = 0;

#ifdef USE_SD_RAW
			if (sd_raw_read_continuous_next())
			{
				free(record);
				return err_file_incomplete_read;
			}
#endif /* USE_SD_RAW */
		}

#if defined(USE_SD_RAW)
		if (sd_raw_read_continuous(record, record_size, record_size * (cursor->block_current_record)))
#elif defined(USE_IONFS)
		if (ionfs_read(file->file_ptr, cursor->block_address, record, record_size, record_size * (cursor->block_current_record)))
#else
		if (device_read(cursor->block_address, record, record_size, record_size * (cursor->block_current_record)))
#endif
		{
			free(record);
			return err_file_incomplete_read;
		}

		if (record->status == END_RECORD)
		{
#ifdef USE_SD_RAW
			if (sd_raw_read_continuous_stop())
			{
				free(record);
				return err_file_incomplete_read;
			}
#endif /* USE_SD_RAW */

			free(record);
			return cs_end_of_results;
		}

		if (record->status != DELETED)
		{
			/**
			 * Compares value == key
			 */
			boolean_t key_satisfies_predicate = ffdict_test_predicate(
					&(cursor->super), (ion_key_t)record->data);		//assumes that the key is first

			if (key_satisfies_predicate == boolean_true)
			{
#ifdef USE_SD_RAW
				if (sd_raw_read_continuous_stop())
				{
					free(record);
					return err_file_incomplete_read;
				}
#endif /* USE_SD_RAW */

				free(record);
				return cs_valid_data;
			}
		}

		(cursor->block_current_record)++;
	} while(record->status != END_RECORD);

#ifdef USE_SD_RAW
	if (sd_raw_read_continuous_stop())
	{
		free(record);
		return err_file_incomplete_read;
	}
#endif /* USE_SD_RAW */

	free(record);
	return cs_end_of_results;
}
