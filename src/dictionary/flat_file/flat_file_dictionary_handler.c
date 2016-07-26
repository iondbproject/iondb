/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Function definitions at the dictionary interface level for the
			flat file store.
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

#include "flat_file_dictionary_handler.h"

void
ffdict_init(
	ion_dictionary_handler_t *handler
) {
	handler->insert				= ffdict_insert;
	handler->create_dictionary	= ffdict_create_dictionary;
	handler->get				= ffdict_query;
	handler->update				= ffdict_update;
	handler->find				= ffdict_find;
	handler->remove				= ffdict_delete;
	handler->delete_dictionary	= ffdict_delete_dictionary;
}

ion_status_t
ffdict_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return ION_STATUS_CREATE(err_not_implemented, 0);
}

ion_status_t
ffdict_query(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return ION_STATUS_CREATE(err_not_implemented, 0);
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
	return err_not_implemented;
}

ion_status_t
ffdict_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
) {
	ION_STATUS_CREATE(err_not_implemented, 0);
}

ion_err_t
ffdict_delete_dictionary(
	ion_dictionary_t *dictionary
) {
	return err_not_implemented;
}

ion_status_t
ffdict_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return ION_STATUS_CREATE(err_not_implemented, 0);
}

ion_err_t
ffdict_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
) {
	return err_not_implemented;
}

ion_cursor_status_t
ffdict_next(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
) {
	return err_not_implemented;
}

void
ffdict_destroy_cursor(
	ion_dict_cursor_t **cursor
) {}

/**
@brief		Checks to see if the given @p key satisfies the predicate stored in @p cursor.
@param		cursor
				Which cursor to test within.
@param		key
				The key under test.
@return		@p boolean_true if the key satisfies the predicate, @p boolean_false otherwise.
*/
ion_boolean_t
ffdict_test_predicate(
	ion_dict_cursor_t	*cursor,
	ion_key_t			key
) {
	return boolean_false;
}
