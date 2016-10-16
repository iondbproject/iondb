/******************************************************************************/
/**
@file
@author		Graeme Douglas, Scott Fazackerley
@see		For more information, refer to @ref dictionary.h.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
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
	char	return_value = 0x73;/* Magic default return value to be easy to spot */

	/*
	 * In this case, the endianness of the process does matter as the code does
	 * a direct comparison of bytes in memory starting for MSB.
	*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

	for (idx = key_size - 1; idx >= 0; idx--) {
#else

	/*@todo This is a potential issue and needs to be tested on SAMD3 */
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
	char	return_value = 0x73;/* Magic default return value to be easy to spot TODO refactor out into macro */

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

	/*@todo This is a potential issue and needs to be tested on SAMD3 */
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

char
dictionary_compare_char_array(
	ion_key_t		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
) {
	return strncmp((char *) first_key, (char *) second_key, key_size);
}

char
dictionary_compare_null_terminated_string(
	ion_key_t		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
) {
	return strncmp((char *) first_key, (char *) second_key, key_size);
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
			return err_dictionary_initialization_failed;
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

		if (cs_end_of_results != cursor_status) {
			return err_dictionary_initialization_failed;
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
			/* TODO not implemented */
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
dictionary_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
) {
	return dictionary->handler->find(dictionary, predicate, cursor);
}

ion_boolean_t
test_predicate(
	ion_dictionary_parent_t parent,
	ion_dict_cursor_t		*cursor,
	ion_key_t				key
) {
	ion_key_size_t	key_size	= cursor->dictionary->instance->record.key_size;
	ion_boolean_t	result		= boolean_false;

	switch (cursor->predicate->type) {
		case predicate_equality: {
			if (parent.compare(key, cursor->predicate->statement.equality.equality_value, cursor->dictionary->instance->record.key_size) == 0) {
				result = boolean_true;
			}

			break;
		}

		case predicate_range: {
			ion_key_t	lower_b			= cursor->predicate->statement.range.lower_bound;
			ion_key_t	upper_b			= cursor->predicate->statement.range.upper_bound;

			/* Check if key >= lower bound */
			ion_boolean_t comp_lower	= parent.compare(key, lower_b, key_size) >= 0;

			/* Check if key <= upper bound */
			ion_boolean_t comp_upper	= parent.compare(key, upper_b, key_size) <= 0;

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
