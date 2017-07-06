/******************************************************************************/
/**
@file		dictionary.c
@author		Graeme Douglas, Scott Fazackerley
@see		For more information, refer to dictionary.h.
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

#include "dictionary.h"
#include "flat_file/flat_file_dictionary_handler.h"

int
dictionary_get_filename(
	ion_dictionary_id_t id,
	char				*ext,
	char				*filename
) {
	return snprintf(filename, ION_MAX_FILENAME_LENGTH, "%d.%s", id, ext);
}

/**
@brief		Compare any two character (byte) arrays. These are not assumed
			to be null-terminated.
@param		first_key
				The first (left) key being compared.
@param		second_key
				The second (right) key being compared.
@param		key_size
				The size of the keys being compared.
@return		The resulting comparison value.
*/
char
dictionary_compare_char_array(
	ion_key_t		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
) {
	return strncmp((char *) first_key, (char *) second_key, key_size);
}

/**
@brief		Compare any two null-terminated strings.
@param		first_key
				The first (left) key being compared.
@param		second_key
				The second (right) key being compared.
@param		key_size
				The (maximum) size of the keys being compared.
@return		The resulting comparison value.
*/
char
dictionary_compare_null_terminated_string(
	ion_key_t		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
) {
	return strncmp((char *) first_key, (char *) second_key, key_size);
}

ion_dictionary_compare_t
dictionary_switch_compare(
	ion_key_type_t key_type
) {
	ion_dictionary_compare_t compare;

	switch (key_type) {
		case key_type_numeric_signed: {
			compare = dictionary_compare_signed_value;
			break;
		}

		case key_type_numeric_unsigned: {
			compare = dictionary_compare_unsigned_value;
			break;
		}

		case key_type_char_array: {
			compare = dictionary_compare_char_array;
			break;
		}

		case key_type_null_terminated_string: {
			compare = dictionary_compare_null_terminated_string;
			break;
		}

		default: {
			/* do something - you must bind the correct comparison function */
			break;
		}
	}

	return compare;
}

ion_err_t
dictionary_create(
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary,
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size
) {
	ion_err_t					err;
	ion_dictionary_compare_t	compare = dictionary_switch_compare(key_type);

	err = handler->create_dictionary(id, key_type, key_size, value_size, dictionary_size, compare, handler, dictionary);

	if (err_ok == err) {
		dictionary->instance->id	= id;
		dictionary->status			= ion_dictionary_status_ok;
	}
	else {
		dictionary->status = ion_dictionary_status_error;
	}

	return err;
}

ion_status_t
dictionary_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return dictionary->handler->insert(dictionary, key, value);
}

ion_status_t
dictionary_get(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return dictionary->handler->get(dictionary, key, value);
}

ion_status_t
dictionary_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return dictionary->handler->update(dictionary, key, value);
}

ion_err_t
dictionary_delete_dictionary(
	ion_dictionary_t *dictionary
) {
	return dictionary->handler->delete_dictionary(dictionary);
}

ion_err_t
dictionary_destroy_dictionary(
	ion_dictionary_handler_t	*handler,
	ion_dictionary_id_t			id
) {
	ion_err_t error = handler->destroy_dictionary(id);

	if (err_not_implemented == error) {
		error = ffdict_destroy_dictionary(id);
	}

	return error;
}

ion_status_t
dictionary_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
) {
	return dictionary->handler->remove(dictionary, key);
}

char
dictionary_compare_unsigned_value(
	ion_key_t		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
) {
	int		idx;
	char	return_value = ION_RETURN_VALUE;

	/*
	 * In this case, the endianness of the process does matter as the code does
	 * a direct comparison of bytes in memory starting for MSB.
	*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

	for (idx = key_size - 1; idx >= 0; idx--) {
#else

	for (idx = 0; idx < key_size; idx++) {
#endif

		ion_byte_t	firstbyte	= *((ion_byte_t *) first_key + idx);
		ion_byte_t	secondbyte	= *((ion_byte_t *) second_key + idx);

		if ((return_value = (firstbyte > secondbyte) - (firstbyte < secondbyte)) != ION_ZERO) {
			return return_value;
		}
	}

	return return_value;
}

char
dictionary_compare_signed_value(
	ion_key_t		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
) {
	int		idx;
	char	return_value = ION_RETURN_VALUE;

	/*
	 * In this case, the endianness of the process does matter as the code does
	 * a direct comparison of bytes in memory starting for MSB.
	*/

/* Start at the MSB */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	idx = key_size - 1;
#else
	idx = 0;
#endif

	ion_byte_t	firstbyte	= *((ion_byte_t *) first_key + idx);
	ion_byte_t	secondbyte	= *((ion_byte_t *) second_key + idx);

	/* Do bit comparison on the sign bit to do positive/negative comparison. Lets us exit early in many cases */
	if ((return_value = (secondbyte >> 7) - (firstbyte >> 7)) != ION_ZERO) {
		return return_value;
	}

/* In this case, we are of the same sign. Do byte-for-byte comparison. */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

	for (; idx >= 0; idx--) {
#else

	for (; idx < key_size; idx++) {
#endif
		firstbyte	= *((ion_byte_t *) first_key + idx);
		secondbyte	= *((ion_byte_t *) second_key + idx);

		if ((return_value = (firstbyte > secondbyte) - (firstbyte < secondbyte)) != ION_ZERO) {
			return return_value;
		}
	}

	return return_value;
}

ion_err_t
dictionary_open(
	ion_dictionary_handler_t		*handler,
	ion_dictionary_t				*dictionary,
	ion_dictionary_config_info_t	*config
) {
	ion_dictionary_compare_t compare	= dictionary_switch_compare(config->type);

	ion_err_t error						= handler->open_dictionary(handler, dictionary, config, compare);

	if (err_not_implemented == error) {
		ion_predicate_t				predicate;
		ion_dict_cursor_t			*cursor = NULL;
		ion_record_t				record;
		ion_dictionary_handler_t	fallback_handler;
		ion_dictionary_t			fallback_dict;
		ion_err_t					err;

		ffdict_init(&fallback_handler);

		ion_dictionary_config_info_t fallback_config = {
			config->id, 0, config->type, config->key_size, config->value_size, 1
		};

		err = dictionary_open(&fallback_handler, &fallback_dict, &fallback_config);

		if (err_ok != err) {
			return err;
		}

		dictionary_build_predicate(&predicate, predicate_all_records);
		err = dictionary_find(&fallback_dict, &predicate, &cursor);

		if (err_ok != err) {
			return err;
		}

		record.key		= alloca(config->key_size);
		record.value	= alloca(config->value_size);

		err				= dictionary_create(handler, dictionary, config->id, config->type, config->key_size, config->value_size, config->dictionary_size);

		if (err_ok != err) {
			return err;
		}

		ion_cursor_status_t cursor_status;

		while (cs_cursor_active == (cursor_status = cursor->next(cursor, &record)) || cs_cursor_initialized == cursor_status) {
			ion_status_t status = dictionary_insert(dictionary, record.key, record.value);

			if (err_ok != status.error) {
				cursor->destroy(&cursor);
				dictionary_close(&fallback_dict);
				dictionary_delete_dictionary(dictionary);
				return status.error;
			}
		}

		if (cursor_status != cs_end_of_results) {
			return err_uninitialized;
		}

		cursor->destroy(&cursor);

		err = dictionary_delete_dictionary(&fallback_dict);

		if (err_ok != err) {
			return err;
		}

		error = err_ok;
	}

	if (err_ok == error) {
		dictionary->status			= ion_dictionary_status_ok;
		dictionary->instance->id	= config->id;
	}
	else {
		dictionary->status = ion_dictionary_status_error;
	}

	return error;
}

ion_err_t
dictionary_close(
	ion_dictionary_t *dictionary
) {
	if (ion_dictionary_status_closed == dictionary->status) {
		return err_ok;
	}

	ion_err_t error = dictionary->handler->close_dictionary(dictionary);

	if (err_not_implemented == error) {
		ion_predicate_t		predicate;
		ion_dict_cursor_t	*cursor = NULL;
		ion_record_t		record;
		ion_err_t			err;

		dictionary_build_predicate(&predicate, predicate_all_records);
		err = dictionary_find(dictionary, &predicate, &cursor);

		if (err_ok != err) {
			return err;
		}

		int				key_size	= dictionary->instance->record.key_size;
		int				value_size	= dictionary->instance->record.value_size;
		ion_key_type_t	key_type	= dictionary->instance->key_type;

		record.key		= alloca(key_size);
		record.value	= alloca(value_size);

		ion_dictionary_handler_t	fallback_handler;
		ion_dictionary_t			fallback_dict;

		ffdict_init(&fallback_handler);

		err = dictionary_create(&fallback_handler, &fallback_dict, dictionary->instance->id, key_type, key_size, value_size, 1);

		if (err_ok != err) {
			return err;
		}

		ion_cursor_status_t cursor_status;

		while (cs_cursor_active == (cursor_status = cursor->next(cursor, &record)) || cs_cursor_initialized == cursor_status) {
			ion_status_t status = dictionary_insert(&fallback_dict, record.key, record.value);

			if (err_ok != status.error) {
				cursor->destroy(&cursor);
				dictionary_delete_dictionary(&fallback_dict);
				return status.error;
			}
		}

		/* Cursor has either reached the end of the result set or there was no
		   result set to traverse, and the cursor remains uninitialized. */
		if ((cs_end_of_results != cursor_status) && (cs_cursor_uninitialized != cursor_status)) {
			return err_uninitialized;
		}

		cursor->destroy(&cursor);

		err = dictionary_close(&fallback_dict);

		if (err_ok != err) {
			return err;
		}

		err = dictionary_delete_dictionary(dictionary);

		if (err_ok != err) {
			return err;
		}

		error = err_ok;
	}

	if (err_ok == error) {
		dictionary->status = ion_dictionary_status_closed;
	}

	return error;
}

/**
@brief		Destroys an equality predicate.
@details	This function should not be called directly. Instead, it is set
			while building the predicate.
@param		predicate
				A pointer to the pointer to the predicate object being
				destroyed.
*/
void
dictionary_destroy_predicate_equality(
	ion_predicate_t **predicate
) {
	if (*predicate != NULL) {
		free((*predicate)->statement.equality.equality_value);
		free(*predicate);
		*predicate = NULL;
	}
}

/**
@brief		Destroys a range predicate.
@details	This function should not be called directly. Instead, it is set
			while building the predicate.
@param		predicate
				A pointer to the pointer to the predicate object being
				destroyed.
*/
void
dictionary_destroy_predicate_range(
	ion_predicate_t **predicate
) {
	if (*predicate != NULL) {
		free((*predicate)->statement.range.upper_bound);
		free((*predicate)->statement.range.lower_bound);
		free(*predicate);
		*predicate = NULL;
	}
}

/**
@brief		Destroys an all records predicate.
@details	This function should not be called directly. Instead, it is set
			while building the predicate.
@param		predicate
				A pointer to the pointer to the predicate object being
				destroyed.
*/
void
dictionary_destroy_predicate_all_records(
	ion_predicate_t **predicate
) {
	if (*predicate != NULL) {
		free(*predicate);
		*predicate = NULL;
	}
}

ion_err_t
dictionary_build_predicate(
	ion_predicate_t			*predicate,
	ion_predicate_type_t	type,
	...
) {
	va_list arg_list;

	va_start(arg_list, type);

	predicate->type = type;

	switch (type) {
		case predicate_equality: {
			ion_key_t key = va_arg(arg_list, ion_key_t);

			predicate->statement.equality.equality_value	= key;
			predicate->destroy								= dictionary_destroy_predicate_equality;
			break;
		}

		case predicate_range: {
			ion_key_t	lower_bound = va_arg(arg_list, ion_key_t);
			ion_key_t	upper_bound = va_arg(arg_list, ion_key_t);

			predicate->statement.range.lower_bound	= lower_bound;
			predicate->statement.range.upper_bound	= upper_bound;
			predicate->destroy						= dictionary_destroy_predicate_range;
			break;
		}

		case predicate_all_records: {
			predicate->destroy = dictionary_destroy_predicate_all_records;
			break;
		}

		case predicate_predicate: {
			return err_invalid_predicate;
		}

		default: {
			return err_invalid_predicate;
			break;
		}
	}

	va_end(arg_list);
	return err_ok;
}

ion_err_t
dictionary_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
) {
	return dictionary->handler->find(dictionary, predicate, cursor);
}

ion_boolean_t
test_predicate(
	ion_dict_cursor_t	*cursor,
	ion_key_t			key
) {
	ion_dictionary_parent_t *parent		= cursor->dictionary->instance;
	ion_key_size_t			key_size	= cursor->dictionary->instance->record.key_size;
	ion_boolean_t			result		= boolean_false;

	switch (cursor->predicate->type) {
		case predicate_equality: {
			if (parent->compare(key, cursor->predicate->statement.equality.equality_value, cursor->dictionary->instance->record.key_size) == 0) {
				result = boolean_true;
			}

			break;
		}

		case predicate_range: {
			ion_key_t	lower_b			= cursor->predicate->statement.range.lower_bound;
			ion_key_t	upper_b			= cursor->predicate->statement.range.upper_bound;

			/* Check if key >= lower bound */
			ion_boolean_t comp_lower	= parent->compare(key, lower_b, key_size) >= 0;

			/* Check if key <= upper bound */
			ion_boolean_t comp_upper	= parent->compare(key, upper_b, key_size) <= 0;

			result = comp_lower && comp_upper;
			break;
		}

		case predicate_all_records: {
			result = boolean_true;
			break;
		}
	}

	return result;
}
