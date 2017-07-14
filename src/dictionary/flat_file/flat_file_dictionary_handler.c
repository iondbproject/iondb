/******************************************************************************/
/**
@file		flat_file_dictionary_handler.c
@author		Eric Huang
@brief		Function definitions at the dictionary interface level for the
			flat file store.
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

#include "flat_file_dictionary_handler.h"

/**
@brief			Fetches the next record to be returned from a cursor that has already been initialized.
@details		The returned record is written back to @p record, and then the cursor is advanced to the next
				record. The returned status code signifies whether or not there are more results to traverse.
				This function should not be called directly, but instead will be bound to the cursor like a method.
@param[in]		cursor
					Which cursor to fetch results from.
@param[out]		record
					An initialized record struct with the @p key and @p value appropriately allocated to fit
					the returned key and value. This function will write back data to the struct.
@return			The resulting status of the operation.
*/
ion_cursor_status_t
ffdict_next(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
) {
	ion_flat_file_t			*flat_file			= (ion_flat_file_t *) cursor->dictionary->instance;
	ion_flat_file_cursor_t	*flat_file_cursor	= (ion_flat_file_cursor_t *) cursor;

	if (cursor->status == cs_cursor_uninitialized) {
		return cursor->status;
	}
	else if (cursor->status == cs_end_of_results) {
		return cursor->status;
	}
	else if ((cursor->status == cs_cursor_initialized) || (cursor->status == cs_cursor_active)) {
		if (cursor->status == cs_cursor_active) {
			ion_flat_file_row_t throwaway_row;
			ion_err_t			err = err_uninitialized;

			switch (cursor->predicate->type) {
				case predicate_equality: {
					err = flat_file_scan(flat_file, flat_file_cursor->current_location + 1, &flat_file_cursor->current_location, &throwaway_row, ION_FLAT_FILE_SCAN_FORWARDS, flat_file_predicate_key_match, cursor->predicate->statement.equality.equality_value);

					break;
				}

				case predicate_range: {
					err = flat_file_scan(flat_file, flat_file_cursor->current_location + 1, &flat_file_cursor->current_location, &throwaway_row, ION_FLAT_FILE_SCAN_FORWARDS, flat_file_predicate_within_bounds, cursor->predicate->statement.range.lower_bound, cursor->predicate->statement.range.upper_bound);

					break;
				}

				case predicate_all_records: {
					err = flat_file_scan(flat_file, flat_file_cursor->current_location + 1, &flat_file_cursor->current_location, &throwaway_row, ION_FLAT_FILE_SCAN_FORWARDS, flat_file_predicate_not_empty);

					break;
				}

				case predicate_predicate: {
					break;
				}
			}

			if (err_file_hit_eof == err) {
				cursor->status = cs_end_of_results;
				return cursor->status;
			}
			else if (err_ok != err) {
				cursor->status = cs_possible_data_inconsistency;
				return cursor->status;
			}
		}
		else {
			/* The status is cs_cursor_initialized */
			cursor->status = cs_cursor_active;
		}

		ion_flat_file_row_t row;
		ion_err_t			err = flat_file_read_row(flat_file, flat_file_cursor->current_location, &row);

		if (err_ok != err) {
			return cs_invalid_index;
		}

		/*Copy both key and value into user provided struct */
		memcpy(record->key, row.key, cursor->dictionary->instance->record.key_size);
		memcpy(record->value, row.value, cursor->dictionary->instance->record.value_size);

		return cursor->status;
	}

	return cs_invalid_cursor;
}

/**
@brief		Destroys and frees the given cursor.
@details	This function should not be called directly, but instead accessed through the interface
			the cursor object.
@param[in]	cursor
				Which cursor to destroy.
*/
void
ffdict_destroy_cursor(
	ion_dict_cursor_t **cursor
) {
	(*cursor)->predicate->destroy(&(*cursor)->predicate);
	free(*cursor);
	*cursor = NULL;
}

/**
@brief		Re-instances a previously created flat file store instance
			and prepares it to be used again.
@param[in]	handler
				A handler that must be bound with the flat file's functions.
@param[in]	dictionary
				A dictionary that is allocated but not initialized. We will
				store the details of the re-instanced flat file store in here.
@param[in]	config
				The configuration parameters previously used by the flat file store
				we are opening. This must either be provided directly if dictionaries
				are being managed manually, or will be provided by the dictionary manager
				that is currently in use.
@param[in]	compare
				The comparison function that will be given by higher layers, based on the
				destined key type.
@return		The resulting status of the operation.
*/
ion_err_t
ffdict_open_dictionary(
	ion_dictionary_handler_t		*handler,
	ion_dictionary_t				*dictionary,
	ion_dictionary_config_info_t	*config,
	ion_dictionary_compare_t		compare
) {
	return ffdict_create_dictionary(config->id, config->type, config->key_size, config->value_size, config->dictionary_size, compare, handler, dictionary);
}

/**
@brief		Closes this flat file store and persists everything to disk
			to be brought back later using @ref dictionary_open.
@param[in]	dictionary
				Which instance of a flat file store to close.
@return		The resuling status of the operation.
*/
ion_err_t
ffdict_close_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_err_t err = flat_file_close((ion_flat_file_t *) dictionary->instance);

	free(dictionary->instance);
	dictionary->instance = NULL;

	if (err_ok != err) {
		return err;
	}

	return err_ok;
}

/**
@brief			Initializes a cursor query and returns an allocated cursor object.
@details		Given a @p predicate that was previously initialized by @ref dictionary_build_predicate,
				Build a cursor object that acts as the iterator for the desired query through the dictionary.
@param[in]		dictionary
					Which dictionary to query on.
@param[in]		predicate
					An allocated, initialized predicate object that defines the parameters of the query.
@param[out]		cursor
					A cursor pointer should be initialized to @p NULL, and then given to this function.
					This function shall allocate appropriate memory and redirect @p cursor to point to
					the allocated memory. **NOTE:** Anything originally pointed to by this cursor will
					effectively be lost, if there is no other reference to said thing.
@return			The resulting status of the operation.
*/
ion_err_t
ffdict_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
) {
	*cursor = malloc(sizeof(ion_flat_file_cursor_t));

	ion_flat_file_t *flat_file = (ion_flat_file_t *) dictionary->instance;

	if (NULL == *cursor) {
		return err_out_of_memory;
	}

	(*cursor)->dictionary	= dictionary;
	(*cursor)->status		= cs_cursor_uninitialized;

	(*cursor)->destroy		= ffdict_destroy_cursor;
	(*cursor)->next			= ffdict_next;

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

			ion_fpos_t			loc			= -1;
			ion_flat_file_row_t row;
			ion_err_t			scan_result = flat_file_scan(flat_file, -1, &loc, &row, ION_FLAT_FILE_SCAN_FORWARDS, flat_file_predicate_key_match, target_key);

			if (err_file_hit_eof == scan_result) {
				/* If this happens, that means the target key doesn't exist */
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else if (err_ok == scan_result) {
				(*cursor)->status = cs_cursor_initialized;

				ion_flat_file_cursor_t *flat_file_cursor = (ion_flat_file_cursor_t *) (*cursor);

				flat_file_cursor->current_location = loc;
				return err_ok;
			}
			else {
				/* File scan hit an error condition */
				return scan_result;
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

			/* Find the first satisfactory key. */
			ion_fpos_t			loc			= -1;
			ion_flat_file_row_t row;
			ion_err_t			scan_result = flat_file_scan(flat_file, -1, &loc, &row, ION_FLAT_FILE_SCAN_FORWARDS, flat_file_predicate_within_bounds, (*cursor)->predicate->statement.range.lower_bound, (*cursor)->predicate->statement.range.upper_bound);

			if (err_file_hit_eof == scan_result) {
				/* This means the returned node is smaller than the lower bound, which means that there are no valid records to return */
				(*cursor)->status = cs_end_of_results;
				return err_ok;
			}
			else if (err_ok == scan_result) {
				(*cursor)->status = cs_cursor_initialized;

				ion_flat_file_cursor_t *flat_file_cursor = (ion_flat_file_cursor_t *) (*cursor);

				flat_file_cursor->current_location = loc;
				return err_ok;
			}
			else {
				/* Scan failed due to external error */
				return scan_result;
			}

			break;
		}

		case predicate_all_records: {
			ion_flat_file_cursor_t *flat_file_cursor	= (ion_flat_file_cursor_t *) (*cursor);

			ion_fpos_t			loc						= -1;
			ion_flat_file_row_t row;
			ion_err_t			scan_result				= flat_file_scan(flat_file, -1, &loc, &row, ION_FLAT_FILE_SCAN_FORWARDS, flat_file_predicate_not_empty);

			if (err_file_hit_eof == scan_result) {
				(*cursor)->status = cs_end_of_results;
			}
			else if (err_ok == scan_result) {
				flat_file_cursor->current_location	= loc;
				(*cursor)->status					= cs_cursor_initialized;
			}
			else {
				/* Scan failure */
				return scan_result;
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

void
ffdict_init(
	ion_dictionary_handler_t *handler
) {
	handler->insert				= ffdict_insert;
	handler->create_dictionary	= ffdict_create_dictionary;
	handler->get				= ffdict_get;
	handler->update				= ffdict_update;
	handler->find				= ffdict_find;
	handler->remove				= ffdict_delete;
	handler->delete_dictionary	= ffdict_delete_dictionary;
	handler->destroy_dictionary = ffdict_destroy_dictionary;
	handler->open_dictionary	= ffdict_open_dictionary;
	handler->close_dictionary	= ffdict_close_dictionary;
}

ion_status_t
ffdict_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return flat_file_insert((ion_flat_file_t *) dictionary->instance, key, value);
}

ion_status_t
ffdict_get(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return flat_file_get((ion_flat_file_t *) dictionary->instance, key, value);
}

ion_err_t
ffdict_create_dictionary(
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size,
	ion_dictionary_compare_t	compare,
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary
) {
	dictionary->instance = malloc(sizeof(ion_flat_file_t));

	if (NULL == dictionary->instance) {
		return err_out_of_memory;
	}

	dictionary->instance->compare	= compare;
	dictionary->instance->type		= dictionary_type_flat_file_t;

	ion_err_t result = flat_file_initialize((ion_flat_file_t *) dictionary->instance, id, key_type, key_size, value_size, dictionary_size);

	if ((err_ok == result) && (NULL != handler)) {
		dictionary->handler = handler;
	}

	return result;
}

ion_status_t
ffdict_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
) {
	return flat_file_delete((ion_flat_file_t *) dictionary->instance, key);
}

ion_err_t
ffdict_delete_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_err_t result = flat_file_destroy((ion_flat_file_t *) dictionary->instance);

	free(dictionary->instance);
	dictionary->instance = NULL;
	return result;
}

ion_err_t
ffdict_destroy_dictionary(
	ion_dictionary_id_t id
) {
	char filename[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(id, "ffs", filename);

	if (0 != fremove(filename)) {
		return err_file_delete_error;
	}

	return err_ok;
}

ion_status_t
ffdict_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return flat_file_update((ion_flat_file_t *) dictionary->instance, key, value);
}
