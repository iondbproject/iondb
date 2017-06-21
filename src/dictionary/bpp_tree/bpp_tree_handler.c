/******************************************************************************/
/**
@file		bpp_tree_handler.c
@author		Graeme Douglas
@brief		The handler for a disk-backed B+ Tree.
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

#include "bpp_tree_handler.h"

void
bpptree_get_filename(
	ion_dictionary_id_t id,
	char				*str
) {
	sprintf(str, "%d.val", id);
}

/**
@brief		Creates an instance of a dictionary.

@details	Creates as instance of a dictionary given a @p key_size and
			@p value_size, in bytes. The @p dictionary_size parameter is
			not used for this implementation, as there is no size bound.
@param		id
				ID of a dictionary that's given to us.
@param		key_type
				The key category given to us.
@param		key_size
				The size of the key in bytes.
@param		value_size
				The size of the value in bytes.
@param		dictionary_size
				The size of the hashmap in discrete units
@param		compare
				Function pointer for the comparison function for the dictionary.
@param		handler
				 THe handler for the specific dictionary being created.
@param		dictionary
				 The pointer declared by the caller that will reference
				 the instance of the dictionary created.
@return		The status of the creation of the dictionary.
*/
ion_err_t
bpptree_create_dictionary(
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size,
	ion_dictionary_compare_t	compare,
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary
) {
	UNUSED(dictionary_size);

/*	if (key_size != sizeof(int)) {
		return err_invalid_initial_size;
	}*/

	ion_bpptree_t	*bpptree;
	ion_bpp_open_t	info;

	bpptree = malloc(sizeof(ion_bpptree_t));

	if (NULL == bpptree) {
		return err_out_of_memory;
	}

	char value_filename[20];

	bpptree_get_filename(id, value_filename);
	bpptree->values.file_handle = ion_fopen(value_filename);

	bpptree->values.next_empty	= ION_FILE_NULL;

	char addr_filename[ION_MAX_FILENAME_LENGTH];

	int actual_filename_length = dictionary_get_filename(id, "bpt", addr_filename);

	if (actual_filename_length >= ION_MAX_FILENAME_LENGTH) {
		return err_uninitialized;
	}

	info.iName		= addr_filename;
	info.keySize	= key_size;
	info.dupKeys	= boolean_false;
	info.sectorSize = 256;
	info.comp		= compare;

	ion_bpp_err_t bErr = b_open(info, &(bpptree->tree));

	if (bErrOk != bErr) {
		return err_uninitialized;
	}

	if (NULL == handler) {
		return err_uninitialized;
	}

	dictionary->instance					= (ion_dictionary_parent_t *) bpptree;
	dictionary->instance->compare			= compare;
	dictionary->instance->key_type			= key_type;
	dictionary->instance->record.key_size	= key_size;
	dictionary->instance->record.value_size = value_size;
	dictionary->instance->type				= dictionary_type_bpp_tree_t;
	dictionary->handler						= handler;

	return err_ok;
}

/**
@brief		Inserts a @p key and @p value into the dictionary.

@param	  dictionary
				The dictionary instance to insert the value into.
@param	  key
				The key to use.
@param	  value
				The value to use.
@return		The status on the insertion of the record.
*/
ion_status_t
bpptree_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	ion_bpptree_t		*bpptree;
	ion_bpp_err_t		bErr;
	ion_err_t			err;
	ion_file_offset_t	offset;

	bpptree = (ion_bpptree_t *) dictionary->instance;

	offset	= ION_FILE_NULL;
	bErr	= b_get(bpptree->tree, key, &offset);

	if (bErrKeyNotFound == bErr) {
		offset = ION_FILE_NULL;
	}

	err = lfb_put(&(bpptree->values), (ion_byte_t *) value, bpptree->super.record.value_size, offset, &offset);

	if (err_ok == err) {
		if (bErrKeyNotFound == bErr) {
			bErr = b_insert(bpptree->tree, key, offset);
		}
		else {
			bErr = b_update(bpptree->tree, key, offset);
		}

		if (bErrOk != bErr) {
			return ION_STATUS_ERROR(err_unable_to_insert);
		}

		return ION_STATUS_OK(1);
	}
	else {
		return ION_STATUS_ERROR(err_unable_to_insert);
	}
}

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
@return		The status of the query.
*/
ion_status_t
bpptree_get(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	ion_bpptree_t		*bpptree;
	ion_file_offset_t	offset;
	ion_file_offset_t	next;
	ion_bpp_err_t		bErr;
	ion_err_t			err;

	bpptree = (ion_bpptree_t *) dictionary->instance;

	bErr	= b_get(bpptree->tree, key, &offset);

	if (bErrOk != bErr) {
		return ION_STATUS_ERROR(err_item_not_found);
	}

	err = lfb_get(&(bpptree->values), offset, bpptree->super.record.value_size, (ion_byte_t *) value, &next);

	if (err_ok == err) {
		return ION_STATUS_OK(1);
	}

	return ION_STATUS_ERROR(err);
}

/**
@brief		Deletes the @p key and associated value from the dictionary
			instance.

@param	  dictionary
				The instance of the dictionary to delete from.
@param	  key
				The key that is to be deleted.
@return		The status of the deletion
*/
ion_status_t
bpptree_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
) {
	ion_bpptree_t		*bpptree;
	ion_bpp_err_t		bErr;
	ion_file_offset_t	offset;
	ion_status_t		status;

	status	= ION_STATUS_INITIALIZE;

	bpptree = (ion_bpptree_t *) dictionary->instance;

	bErr	= b_delete(bpptree->tree, key, &offset);

	if (bErrKeyNotFound != bErr) {
		status.error = lfb_delete_all(&(bpptree->values), offset, &(status.count));
	}
	else {
		status.error = err_item_not_found;
	}

	return status;
}

/**
@brief			Closes a BppTree instance of a dictionary.

@param			dictionary
					A pointer to the specific dictionary instance to be closed.

@return			The status of closing the dictionary.
 */
ion_err_t
bpptree_close_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_bpptree_t	*bpptree;
	ion_bpp_err_t	bErr;

	bpptree					= (ion_bpptree_t *) dictionary->instance;
	bErr					= b_close(bpptree->tree);
	ion_fclose(bpptree->values.file_handle);
	free(dictionary->instance);
	dictionary->instance	= NULL;

	if (bErrOk != bErr) {
		return err_dictionary_destruction_error;
	}

	return err_ok;
}

/**
@brief	  Deletes an instance of the dictionary and associated data.

@param	  dictionary
				The instance of the dictionary to delete.
@return		The status of the dictionary deletion.
*/
ion_err_t
bpptree_delete_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_err_t error;

	char	addr_filename[ION_MAX_FILENAME_LENGTH];
	char	value_filename[ION_MAX_FILENAME_LENGTH];

	int actual_addr_filename_length		= dictionary_get_filename(dictionary->instance->id, "bpt", addr_filename);
	int actual_value_filename_length	= dictionary_get_filename(dictionary->instance->id, "val", value_filename);

	if ((actual_addr_filename_length >= ION_MAX_FILENAME_LENGTH) || (actual_value_filename_length >= ION_MAX_FILENAME_LENGTH)) {
		return err_dictionary_destruction_error;
	}

	error = bpptree_close_dictionary(dictionary);

	if (err_ok != error) {
		return error;
	}

	ion_fremove(addr_filename);
	ion_fremove(value_filename);

	return err_ok;
}

/**
@brief	  Deletes a closed instance of the dictionary and associated data.

@param	  id
				The identifier identifying the dictionary to delete.
@return		The status of the dictionary deletion.
*/
ion_err_t
bpptree_destroy_dictionary(
	ion_dictionary_id_t id
) {
	char	addr_filename[ION_MAX_FILENAME_LENGTH];
	char	value_filename[ION_MAX_FILENAME_LENGTH];

	int actual_addr_filename_length		= dictionary_get_filename(id, "bpt", addr_filename);
	int actual_value_filename_length	= dictionary_get_filename(id, "val", value_filename);

	if ((actual_addr_filename_length >= ION_MAX_FILENAME_LENGTH) || (actual_value_filename_length >= ION_MAX_FILENAME_LENGTH)) {
		return err_dictionary_destruction_error;
	}

	ion_fremove(addr_filename);
	ion_fremove(value_filename);

	return err_ok;
}

/**
@brief		Updates the value for a given key.

@details	Updates the value for a given @p key.  If the key does not currently
			exist in the hashmap, it will be created and the value sorted.

@param	  dictionary
				The instance of the dictionary to be updated.
@param	  key
				The key that is to be updated.
@param	  value
				The value that is to be updated.
@return		The status of the update.
*/
ion_status_t
bpptree_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	ion_bpptree_t		*bpptree;
	ion_bpp_err_t		bErr;
	ion_file_offset_t	offset;
	ion_result_count_t	count;

	count	= 0;
	bpptree = (ion_bpptree_t *) dictionary->instance;

	bErr	= b_get(bpptree->tree, key, &offset);

	if (bErrKeyNotFound != bErr) {
		lfb_update_all(&(bpptree->values), offset, bpptree->super.record.value_size, (ion_byte_t *) value, &count);
	}
	else {
		return bpptree_insert(dictionary, key, value);
	}

	return ION_STATUS_OK(count);
}

/**
@brief		Next function to query and retrieve the next
			<K,V> that stratifies the predicate of the cursor.

@param		cursor
				The cursor to iterate over the results.
@param		record
				The structure used to hold the returned key value
				pair. This must be properly initialized and allocated
				by the user.
@return		The status of the cursor.
*/
ion_cursor_status_t
bpptree_next(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
) {
	ion_bpp_cursor_t	*bCursor	= (ion_bpp_cursor_t *) cursor;
	ion_bpptree_t		*bpptree	= (ion_bpptree_t *) cursor->dictionary->instance;

	if (cursor->status == cs_cursor_uninitialized) {
		return cursor->status;
	}
	else if (cursor->status == cs_end_of_results) {
		return cursor->status;
	}
	else if ((cursor->status == cs_cursor_initialized) || (cursor->status == cs_cursor_active)) {
		if (cursor->status == cs_cursor_active) {
			ion_boolean_t is_valid = boolean_true;

			switch (cursor->predicate->type) {
				case predicate_equality: {
					if (-1 == bCursor->offset) {
						/* End of results, we can quit */
						is_valid = boolean_false;
					}

					break;
				}

				case predicate_range: {
					/*do b_find_next_key then test_predicate */
					if (-1 == bCursor->offset) {
						ion_bpp_err_t bErr = b_find_next_key(bpptree->tree, bCursor->cur_key, &bCursor->offset);

						if ((bErrOk != bErr) || (boolean_false == test_predicate(cursor, bCursor->cur_key))) {
							is_valid = boolean_false;
						}
					}

					break;
				}

				case predicate_all_records: {
					if (-1 == bCursor->offset) {
						ion_bpp_err_t bErr = b_find_next_key(bpptree->tree, bCursor->cur_key, &bCursor->offset);

						if (bErrOk != bErr) {
							is_valid = boolean_false;
						}
					}

					break;
				}

				case predicate_predicate: {
					break;
				}
					/*No default since we can assume the predicate is valid. */
			}

			if (boolean_false == is_valid) {
				cursor->status = cs_end_of_results;
				return cursor->status;
			}
		}
		else {
			/* The status is cs_cursor_initialized */
			cursor->status = cs_cursor_active;
		}

		/* Get key */
		memcpy(record->key, bCursor->cur_key, cursor->dictionary->instance->record.key_size);

		/* Get value */
		lfb_get(&(bpptree->values), bCursor->offset, cursor->dictionary->instance->record.value_size, record->value, &bCursor->offset);
		return cursor->status;
	}

	return cs_invalid_cursor;
}

/**
@brief		Destroys the cursor.

@details	Destroys the cursor when the user is finished with it.  The
			destroy function will free up internally allocated memory as well
			as freeing up any reference to the cursor itself.  Cursor pointers
			will be set to NULL as per ION_DB specification for de-allocated
			pointers.

@param	  cursor
				** pointer to cursor.
*/
void
bpptree_destroy_cursor(
	ion_dict_cursor_t **cursor
) {
	(*cursor)->predicate->destroy(&(*cursor)->predicate);
	free(((ion_bpp_cursor_t *) (*cursor))->cur_key);
	free((*cursor));
	*cursor = NULL;
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
@return		The status of the operation.
*/

ion_err_t
bpptree_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
) {
	ion_bpptree_t	*bpptree	= (ion_bpptree_t *) dictionary->instance;
	ion_key_size_t	key_size	= dictionary->instance->record.key_size;

	*cursor = malloc(sizeof(ion_bpp_cursor_t));

	if (NULL == *cursor) {
		return err_out_of_memory;
	}

	ion_bpp_cursor_t *bCursor = (ion_bpp_cursor_t *) (*cursor);

	bCursor->cur_key = malloc(key_size);

	if (NULL == bCursor->cur_key) {
		free(bCursor);
		return err_out_of_memory;
	}

	(*cursor)->dictionary	= dictionary;
	(*cursor)->status		= cs_cursor_uninitialized;

	(*cursor)->destroy		= bpptree_destroy_cursor;
	(*cursor)->next			= bpptree_next;

	(*cursor)->predicate	= malloc(sizeof(ion_predicate_t));

	if (NULL == (*cursor)->predicate) {
		free(bCursor->cur_key);
		free(*cursor);
		return err_out_of_memory;
	}

	(*cursor)->predicate->type		= predicate->type;
	(*cursor)->predicate->destroy	= predicate->destroy;

	switch (predicate->type) {
		case predicate_equality: {
			ion_key_t target_key = predicate->statement.equality.equality_value;

			(*cursor)->predicate->statement.equality.equality_value = malloc(key_size);

			if (NULL == (*cursor)->predicate->statement.equality.equality_value) {
				free((*cursor)->predicate);
				free(bCursor->cur_key);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy((*cursor)->predicate->statement.equality.equality_value, target_key, key_size);

			memcpy(bCursor->cur_key, target_key, key_size);

			ion_bpp_err_t err = b_get(bpptree->tree, target_key, &bCursor->offset);

			if (bErrOk != err) {
				/* If this happens, that means the target key doesn't exist */
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else {
				(*cursor)->status = cs_cursor_initialized;
				return err_ok;
			}

			break;
		}

		case predicate_range: {
			(*cursor)->predicate->statement.range.lower_bound = malloc(key_size);

			if (NULL == (*cursor)->predicate->statement.range.lower_bound) {
				free((*cursor)->predicate);
				free(bCursor->cur_key);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy((*cursor)->predicate->statement.range.lower_bound, predicate->statement.range.lower_bound, key_size);

			(*cursor)->predicate->statement.range.upper_bound = malloc(key_size);

			if (NULL == (*cursor)->predicate->statement.range.upper_bound) {
				free((*cursor)->predicate->statement.range.lower_bound);
				free((*cursor)->predicate);
				free(bCursor->cur_key);
				free(*cursor);
				return err_out_of_memory;
			}

			memcpy((*cursor)->predicate->statement.range.upper_bound, predicate->statement.range.upper_bound, key_size);

			/* We search for the FGEQ of the Lower bound. */
			b_find_first_greater_or_equal(bpptree->tree, (*cursor)->predicate->statement.range.lower_bound, bCursor->cur_key, &bCursor->offset);

			/* If the key returned doesn't satisfy the predicate, we can exit */
			if (boolean_false == test_predicate(*cursor, bCursor->cur_key)) {
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else {
				(*cursor)->status = cs_cursor_initialized;
				return err_ok;
			}

			break;
		}

		case predicate_all_records: {
			ion_bpp_err_t err;

			/* We search for first key in B++ tree. */
			err					= b_find_first_key(bpptree->tree, bCursor->cur_key, &bCursor->offset);

			(*cursor)->status	= cs_cursor_initialized;

			if (bErrOk != err) {
				(*cursor)->status = cs_end_of_results;
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
@brief			Opens a specific BppTree instance of a dictionary.

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
bpptree_open_dictionary(
	ion_dictionary_handler_t		*handler,
	ion_dictionary_t				*dictionary,
	ion_dictionary_config_info_t	*config,
	ion_dictionary_compare_t		compare
) {
	return bpptree_create_dictionary(config->id, config->type, config->key_size, config->value_size, config->dictionary_size, compare, handler, dictionary);
}

void
bpptree_init(
	ion_dictionary_handler_t *handler
) {
	handler->insert				= bpptree_insert;
	handler->create_dictionary	= bpptree_create_dictionary;
	handler->get				= bpptree_get;
	handler->update				= bpptree_update;
	handler->find				= bpptree_find;
	handler->remove				= bpptree_delete;
	handler->delete_dictionary	= bpptree_delete_dictionary;
	handler->destroy_dictionary = bpptree_destroy_dictionary;
	handler->open_dictionary	= bpptree_open_dictionary;
	handler->close_dictionary	= bpptree_close_dictionary;
}
