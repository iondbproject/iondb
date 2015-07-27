/******************************************************************************/
/**
@file 		bpptreehandler.c
@author		Graeme Douglas
@brief		The handler for a disk-backed B+ Tree.
 */
/******************************************************************************/

#include "bpptreehandler.h"
#include "./../../kv_system.h"
#include "./../../kv_io.h"

void
bpptree_get_addr_filename(
	ion_dictionary_id_t id,
	char 				*str
)
{
	sprintf(str, "%d.bpt", id);
	
}

void
bpptree_get_value_filename(
	ion_dictionary_id_t id,
	char 				*str
)
{
	sprintf(str, "%d.val", id);
}

void
bpptree_init(
	dictionary_handler_t 	*handler
)
{
	handler->insert 			= bpptree_insert;
	handler->create_dictionary 	= bpptree_create_dictionary;
	handler->get 				= bpptree_query;
	handler->update 			= bpptree_update;
	handler->find 				= bpptree_find;
	handler->remove 			= bpptree_delete;
	handler->delete_dictionary 	= bpptree_delete_dictionary;
	handler->open_dictionary 	= bpptree_open_dictionary;
	handler->close_dictionary 	= bpptree_close_dictionary;
}

err_t
bpptree_insert(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t		value
)
{
	bpptree_t		*bpptree;
	bErrType		bErr;
	err_t			err;
	file_offset_t		offset;
		
	bpptree	= (bpptree_t *) dictionary->instance;
	
	offset	= FILE_NULL;
	bErr	= bFindKey(bpptree->tree, key, &offset);
	if (bErrKeyNotFound == bErr)
	{
		offset	= FILE_NULL;
	}
	
	err	= lfb_put(
			&(bpptree->values),
			(byte *)value,
			bpptree->super.record.value_size,
			offset,
			&offset
		);
	
	if (err_ok == err)
	{
		if (bErrKeyNotFound == bErr)
		{
			bErr	= bInsertKey(bpptree->tree, key, offset);
		}
		else
		{
			bErr	= bUpdateKey(bpptree->tree, key, offset);
		}
		if (bErrOk != bErr)
		{
			// TODO: lfb_delete from values
			return err_could_not_insert;
		}
		return err_ok;
	}
	else
	{
		return err_could_not_insert;
	}
}

err_t
bpptree_query(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t		value
)
{
	bpptree_t		*bpptree;
	file_offset_t		offset;
	file_offset_t		next;
	bErrType		bErr;
	err_t			err;
	
	bpptree	= (bpptree_t *) dictionary->instance;
	
	bErr	= bFindKey(bpptree->tree, key, &offset);
	
	if (bErrOk != bErr)
	{
		return err_item_not_found;
	}
	
	err	= lfb_get(
			&(bpptree->values),
			offset,
			bpptree->super.record.value_size,
			(byte *) value,
			&next
		);
	
	return err;
}

err_t
bpptree_create_dictionary(
		ion_dictionary_id_t 		id,
		key_type_t					key_type,
		int 						key_size,
		int 						value_size,
		int 						dictionary_size,
		ion_dictionary_compare_t 	compare,
		dictionary_handler_t 		*handler,
		dictionary_t 				*dictionary
)
{
	bpptree_t				*bpptree;
	bErrType				bErr;
	bOpenType				info;

	bpptree					= malloc(sizeof(bpptree_t));
	if (NULL == bpptree)
	{
		return err_out_of_memory;
	}

	char value_filename[20];
	bpptree_get_value_filename(id, value_filename);
	bpptree->values.file_handle		= ion_fopen(value_filename);
	
	bpptree->values.next_empty		= FILE_NULL;
		// FIXME: read this from a property bag.
	
	// FIXME: VARIABLE NAMES!
	char addr_filename[20];
	bpptree_get_addr_filename(id, addr_filename);
	info.iName					= addr_filename;
	info.keySize				= key_size;
	info.dupKeys				= boolean_false;
	// FIXME: HOW DO WE SET BLOCK SIZE?
	info.sectorSize				= 256;
	info.comp				= compare;
	
	if (bErrOk != (bErr = bOpen(info, &(bpptree->tree))))
	{
		return err_dictionary_initialization_failed;
	}

	dictionary->instance						= (dictionary_parent_t*) bpptree;
	dictionary->instance->compare 				= compare;
	dictionary->instance->key_type 				= key_type;
	dictionary->instance->record.key_size 		= key_size;
	dictionary->instance->record.value_size 	= value_size;
	//todo: need to check to make sure that the handler is registered
	dictionary->handler							= handler;		

	return err_ok;
}

err_t
bpptree_delete(
		dictionary_t 	*dictionary,
		ion_key_t 		key
)
{
	bpptree_t		*bpptree;
	bErrType		bErr;
	file_offset_t		offset;
	
	bpptree	= (bpptree_t *) dictionary->instance;
	
	bErr	= bDeleteKey(bpptree->tree, key, &offset);
	if (bErrKeyNotFound != bErr)
	{
		return lfb_delete_all(&(bpptree->values), offset);
	}
	
	return err_ok;
}

err_t
bpptree_delete_dictionary(
		dictionary_t 	*dictionary
)
{
	err_t error;
	
	char addr_filename[20];
	char value_filename[20];
	bpptree_get_addr_filename(dictionary->instance->id, addr_filename);
	bpptree_get_value_filename(dictionary->instance->id, value_filename);

	error = bpptree_close_dictionary(dictionary);

	if (err_ok != error) { return error; }

	ion_fremove(addr_filename);
	ion_fremove(value_filename);
	
	return err_ok;
}

err_t
bpptree_update(
		dictionary_t 	*dictionary,
		ion_key_t 		key,
		ion_value_t 	value
)
{
	bpptree_t		*bpptree;
	bErrType		bErr;
	file_offset_t		offset;
	
	bpptree	= (bpptree_t *) dictionary->instance;
	
	bErr	= bFindKey(bpptree->tree, key, &offset);
	if (bErrKeyNotFound != bErr)
	{
		lfb_update_all(
			&(bpptree->values),
			offset,
			bpptree->super.record.value_size,
			(byte *)value
		);
	}
	else
		return bpptree_insert(dictionary, key, value);
	
	return err_ok;
}

err_t
bpptree_find(
		dictionary_t 	*dictionary,
		predicate_t 	*predicate,
		dict_cursor_t 	**cursor
)
{
	bpptree_t *bpptree = (bpptree_t *) dictionary->instance;
	ion_key_size_t 	key_size 	= dictionary->instance->record.key_size;

	*cursor 					= malloc(sizeof(bCursorType));
	if(NULL == *cursor) { return err_out_of_memory; }
	bCursorType *bCursor = (bCursorType *) (*cursor);
	bCursor->cur_key 			= malloc(key_size);
	if(NULL == bCursor->cur_key)
	{
		free(bCursor);
		return err_out_of_memory;
	}

	(*cursor)->dictionary 		= dictionary;
	(*cursor)->status 			= cs_cursor_uninitialized;

	(*cursor)->destroy 			= bpptree_destroy_cursor;
	(*cursor)->next 			= bpptree_next;

	(*cursor)->predicate 		= malloc(sizeof(predicate_t));
	if(NULL == (*cursor)->predicate)
	{
		free(bCursor->cur_key);
		free(*cursor);
		return err_out_of_memory;
	}
	(*cursor)->predicate->type 		= predicate->type;
	(*cursor)->predicate->destroy 	= predicate->destroy;



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
				free(bCursor->cur_key);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy(
				(*cursor)->predicate->statement.equality.equality_value,
				target_key,
				key_size
			);

			memcpy(bCursor->cur_key, target_key, key_size);
			
			bErrType err = bFindKey(bpptree->tree, target_key, &bCursor->offset);

			if(bErrOk != err)
			{
				/* If this happens, that means the target key doesn't exist */
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else
			{
				(*cursor)->status 			= cs_cursor_initialized;
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
				free(bCursor->cur_key);
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
				free(bCursor->cur_key);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy(
				(*cursor)->predicate->statement.range.geq_value,
				predicate->statement.range.geq_value,
				key_size
			);

			/* We search for the FGEQ of the Lower bound. */
			bFindFirstGreaterOrEqual(
			                         bpptree->tree,
			                         (*cursor)->predicate->statement.range.leq_value,
			                         bCursor->cur_key,
			                         &bCursor->offset
			                        );

			/* If the key returned doesn't satisfy the predicate, we can exit */
			if(boolean_false == bpptree_test_predicate(*cursor, bCursor->cur_key))
			{
				(*cursor)->status 	= cs_end_of_results;
				return err_ok;
			}
			else
			{
				(*cursor)->status 			= cs_cursor_initialized;
				return err_ok;
			}
			break;
		}
		case predicate_all_records:
		{
			bErrType err;

			/* We search for first key in B++ tree. */
			err	= bFindFirstKey(
					bpptree->tree,
					bCursor->cur_key,
					&bCursor->offset
				);
			
			(*cursor)->status				= cs_cursor_initialized;
			if (bErrOk != err)
			{
				(*cursor)->status 			= cs_end_of_results;
			}
			return err_ok;
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
bpptree_next(
	dict_cursor_t 	*cursor,
	ion_record_t	*record
)
{
	bCursorType *bCursor 	= (bCursorType *) cursor;
	bpptree_t *bpptree = (bpptree_t *) cursor->dictionary->instance;

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
			boolean_t 	is_valid = boolean_true;
			switch(cursor->predicate->type)
			{
				case predicate_equality:
				{
					if(-1 == bCursor->offset)
					{
						/* End of results, we can quit */
						is_valid = boolean_false;
					}
					break;
				}
				case predicate_range:
				{
					/*do bFindNextKey then test_predicate */
					if (-1 == bCursor->offset)
					{
						bErrType bErr = bFindNextKey(bpptree->tree, bCursor->cur_key, &bCursor->offset);
						if (bErrOk != bErr || boolean_false == bpptree_test_predicate(cursor, bCursor->cur_key))
						{
							is_valid = boolean_false;
						}
					}
					break;
				}
				case predicate_all_records:
				{
					if (-1 == bCursor->offset)
					{
						bErrType bErr = bFindNextKey(bpptree->tree, bCursor->cur_key, &bCursor->offset);
						if (bErrOk != bErr)
						{
							is_valid = boolean_false;
						}
					}
				}
				case predicate_predicate:
				{
					/*TODO Not implemented */
				}
				/*No default since we can assume the predicate is valid. */
			}

			if(boolean_false == is_valid)
			{
				cursor->status 	= cs_end_of_results;
				return cursor->status;
			}
		}
		else /* The status is cs_cursor_initialized */
		{
			cursor->status 	= cs_cursor_active;
		}

		/* Get key */
		memcpy(
	       record->key,
	       bCursor->cur_key,
	       cursor->dictionary->instance->record.key_size
		);

		/* Get value */
	    lfb_get(
            &(bpptree->values),
            bCursor->offset,
            cursor->dictionary->instance->record.value_size,
            record->value,
            &bCursor->offset
        );
		return cursor->status;
	}

	return cs_invalid_cursor;
}

void
bpptree_destroy_cursor(
	dict_cursor_t	 **cursor
)
{
	(*cursor)->predicate->destroy(&(*cursor)->predicate);
	free( (( bCursorType *) (*cursor))->cur_key);
	free( (*cursor));
	*cursor = NULL;
}

boolean_t
bpptree_test_predicate(
    dict_cursor_t		*cursor,
    ion_key_t 			key
)
{
	bpptree_t 		*bpptree 	= (bpptree_t *) cursor->dictionary->instance;
	ion_key_size_t 	key_size	= cursor->dictionary->instance->record.key_size;
	boolean_t 		result 		= boolean_false;

	switch(cursor->predicate->type)
	{
		case predicate_equality:
		{
			if(bpptree->super.compare(
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
		case predicate_range:
		{
			ion_key_t lower_b 	= cursor->predicate->statement.range.leq_value;
			ion_key_t upper_b 	= cursor->predicate->statement.range.geq_value;

			/* Check if key >= lower bound */
			boolean_t comp_lower = bpptree->super.compare(key, lower_b, key_size) >= 0;

			/* Check if key <= upper bound */
			boolean_t comp_upper = bpptree->super.compare(key, upper_b, key_size) <= 0;

			result = comp_lower && comp_upper;
			break;
		}
	}

	return result;
}

err_t
bpptree_open_dictionary(
	dictionary_handler_t 			*handler,
	dictionary_t 					*dictionary,
	ion_dictionary_config_info_t 	*config,
	ion_dictionary_compare_t	 	compare
)
{
	err_t error;

	error = bpptree_create_dictionary(
		config->id,
		config->type,
		config->key_size,
		config->value_size,
		config->dictionary_size,
		compare,
		handler,
		dictionary
	);
	if (err_ok != error) { return error; }

	return err_ok;
}

err_t
bpptree_close_dictionary(
	dictionary_t 					*dictionary
)
{
	bpptree_t		*bpptree;
	bErrType		bErr;

	bpptree			= (bpptree_t *) dictionary->instance;
	bErr			= bClose(bpptree->tree);
	ion_fclose(bpptree->values.file_handle);
	free(dictionary->instance);
	dictionary->instance	= NULL;
	
	if (bErrOk != bErr)
	{
		return err_colllection_destruction_error;
	}
	
	return err_ok;
}
