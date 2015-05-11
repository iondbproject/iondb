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
		bErr	= bInsertKey(bpptree->tree, key, offset);
		if (bErrOk != bErr)
		{
			// TODO: lfb_delete from values
			printf("berr %d\n", bErr);
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
		key_type_t				key_type,
		int 					key_size,
		int 					value_size,
		int 					dictionary_size,
		char					(* compare)(ion_key_t, ion_key_t, ion_key_size_t),
		dictionary_handler_t 	*handler,
		dictionary_t 			*dictionary
)
{
	bpptree_t		*bpptree;
	bErrType		bErr;
	bOpenType		info;
	
	bpptree					= malloc(sizeof(bpptree_t));
	if (NULL == bpptree)
		return err_out_of_memory;
	
	bpptree->super.record.key_size		= key_size;
	bpptree->super.record.value_size	= value_size;
	bpptree->values.file_handle		= ion_fopen("FIXME.values");
	bpptree->values.next_empty		= FILE_NULL;
		// FIXME: read this from a property bag.
	
	// FIXME: VARIABLE NAMES!
	info.iName				= "FIXME.bpptree";
	info.keySize				= key_size;
	info.dupKeys				= boolean_false;
	// FIXME: HOW DO WE SET BLOCK SIZE?
	info.sectorSize				= 256;
	info.comp				= compare;
	
	if (bErrOk != (bErr = bOpen(info, &(bpptree->tree))))
	{
		return err_dictionary_initialization_failed;
	}
	
	dictionary->instance		= (dictionary_parent_t*) bpptree;
	
	dictionary->handler		= handler;		//todo: need to check to make sure that the handler is registered

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
	bpptree_t		*bpptree;
	bErrType		bErr;
	
	bpptree			= (bpptree_t *) dictionary->instance;
	bErr			= bClose(bpptree->tree);
	ion_fclose(bpptree->values.file_handle);
	free(dictionary->instance);
	dictionary->instance	= NULL;
	
	// FIXME: Support multiple trees.
	ion_fremove("FIXME.bpptree");
	ion_fremove("FIXME.values");
	
	if (bErrOk != bErr)
	{
		return err_colllection_destruction_error;
	}
	
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

/** @todo What do we do if the cursor is already active? */
err_t
bpptree_find(
		dictionary_t 	*dictionary,
		predicate_t 	*predicate,
		dict_cursor_t 	**cursor
)
{
	bpptree_t *bpptree = (bpptree_t *) dictionary->instance;

	*cursor 					= malloc(sizeof(bCursorType));
	if(NULL == *cursor) { return err_out_of_memory; }
	bCursorType *bCursor = (bCursorType *) (*cursor);

	(*cursor)->dictionary 		= dictionary;
	(*cursor)->type 			= predicate->type;
	(*cursor)->status 			= cs_cursor_uninitialized;

	(*cursor)->destroy 			= bpptree_destroy_cursor;
	(*cursor)->next 			= bpptree_next;

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
			
			bErrType err = bFindKey(bpptree->tree, target_key, (eAdrType *) &bCursor->offset);

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

			/* B+ Tree cursor setup stuff here */

			if(0 /* Bad cursor conditional */)
			{
				(*cursor)->status 	= cs_end_of_results;
				return err_ok;
			}
			else
			{
				(*cursor)->status 			= cs_cursor_initialized;
				/* setup good cursor */
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
bpptree_next(
	dict_cursor_t 	*cursor,
	ion_record_t	*record
)
{
#if 0
	// @todo if the collection changes, then the status of the cursor needs to change
	bpptree_cursor_t *oadict_cursor = (bpptree_cursor_t *)cursor;

	//check the status of the cursor and if it is not valid or at the end, just exit
	if (cursor->status == cs_cursor_uninitialized)
		return cursor->status;
	else if (cursor->status == cs_end_of_results)
		return cursor->status;
	else if ((cursor->status == cs_cursor_initialized )
				|| (cursor->status == cs_cursor_active))	//cursor is active and results have never been accessed
	{
		//extract reference to map
		hashmap_t *hash_map 	= ((hashmap_t*)cursor->dictionary->instance);

		//assume that the value has been pre-allocated
		//compute length of data record stored in map
		int data_length 		= hash_map->super.record.key_size
											+ hash_map->super.record.value_size;

		if (cursor->status == cs_cursor_active)			//find the next valid entry
		{
			//scan and determine what to do?
			if (cs_end_of_results == oadict_scan(oadict_cursor))
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

		//the results are now ready //reference item at given position
		hash_bucket_t * item	= (((hash_bucket_t *)((hash_map->entry
										+ (data_length + SIZEOF(STATUS)) * oadict_cursor->current/*idx*/))));

		//and copy value in
		memcpy(value, (ion_value_t)(item->data+hash_map->super.record.key_size),hash_map->super.record.value_size);

		//and update current cursor position
		return cursor->status;
	}
	//and if you get this far, the cursor is invalid
#endif
	return cs_invalid_cursor;
}

boolean_t
bpptree_is_equal(
	dictionary_t 	*dict,
	ion_key_t 		key1,
	ion_key_t 		key2
)
{
	if (memcmp(key1, key2, (((bpptree_t*)dict->instance)->super.record.key_size)) == IS_EQUAL)
		return boolean_true;
	else
		return boolean_false;
}

void
bpptree_destroy_cursor(
	dict_cursor_t	 **cursor
)
{
}

boolean_t
bpptree_test_predicate(
    dict_cursor_t		*cursor,
    ion_key_t 			key
)
{
#if 0
	//need to check key match; what's the most efficient way?
	//need to use fnptr here for the correct matching
	/*int key_is_eq ual = memcmp(item->data, "10{" ,
	 hash_map->record.key_size);*/
	/**
	 * Compares value == key
	 */
	int key_satisfies_predicate;
	hashmap_t * hash_map = (hashmap_t *)(cursor->dictionary->instance);

	//pre-prime value for faster exit
	key_satisfies_predicate = boolean_false;

	switch (cursor->type)
	{
		case cursor_equality: //equality scan check
		{
			if (IS_EQUAL ==	hash_map->compare(
								cursor->predicate->statement.equality.equality_value,
								key,
								hash_map->super.record.key_size)
			)
			{
				key_satisfies_predicate = boolean_true;
			}
			break;
		}
		case cursor_range: // range check
		{
			if (		// leq_value <= key <==> !(leq_value > key)
					(
						!( A_gt_B == hash_map->compare(
									key,
									cursor->predicate->statement.range.leq_value,
			                		hash_map->super.record.key_size)
						)
					)
					&&	// key <= geq_value <==> !(key > geq_key)
					(
						!(A_gt_B == hash_map->compare(
									cursor->predicate->statement.range.geq_value,
									key,
			                       hash_map->super.record.key_size)
						)
					)
				)
			{
				key_satisfies_predicate = boolean_true;
			}
			break;
		}
	}
	return key_satisfies_predicate;
#endif
	return boolean_false;
}
