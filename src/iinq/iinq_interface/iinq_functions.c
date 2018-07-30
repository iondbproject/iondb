/******************************************************************************/
/**
@file		iinq_functions.c
@author		Dana Klamut
@brief		This code contains definitions for iinq pre-defined functions
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

#include "iinq_functions.h"
#include "../../util/sort/sort.h"

ion_err_t
iinq_execute(
	iinq_table_id_t			table_id,
	ion_key_t				key,
	ion_value_t				value,
	iinq_operation_type_t	type
) {
	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;
	ion_dict_cursor_t			*cursor = NULL;
	ion_predicate_t				predicate;

	dictionary.handler	= &handler;
	error				= iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != error) {
		goto ERROR;
	}

	dictionary_build_predicate(&predicate, predicate_equality, key);
	error = dictionary_find(&dictionary, &predicate, &cursor);

	if (err_ok != error) {
		goto ERROR;
	}

	if (cs_end_of_results != cursor->status) {
		error = err_duplicate_key;
		goto ERROR;
	}

	ion_status_t status;

	switch (type) {
		case iinq_insert_t:
			status	= dictionary_insert(&dictionary, key, value);
			error	= status.error;
			break;

		case iinq_delete_t:
			status	= dictionary_delete(&dictionary, key);
			error	= status.error;
			break;

		case iinq_update_t:
			status	= dictionary_update(&dictionary, key, value);
			error	= status.error;
			break;
	}

	if (err_ok != error) {
		goto ERROR;
	}

ERROR:

	if (NULL != cursor) {
		cursor->destroy(&cursor);
	}

	if (err_ok == error) {
		error = ion_close_dictionary(&dictionary);
	}
	else {
		ion_close_dictionary(&dictionary);
	}

	return error;
}

ion_boolean_t
where(
	iinq_table_id_t		table_id,
	ion_record_t		*record,
	int					num_wheres,
	iinq_where_params_t *where
) {
	int i;

	iinq_where_params_t iinq_where;

	for (i = 0; i < num_wheres; i++) {
		iinq_where = where[i];

		unsigned char *curr						= record->value;

		iinq_field_t	field_type				= iinq_get_field_type(table_id, iinq_where.where_field);
		int				int_value				= 0;
		char			*char_value				= NULL;
		unsigned char	*unsigned_char_value	= NULL;

		switch (field_type) {
			case iinq_int:
				int_value = NEUTRALIZE(iinq_where.field_value, int);
				break;

			case iinq_null_terminated_string:
			case iinq_char_array:
				char_value = iinq_where.field_value;
				break;
		}

		curr = curr + iinq_calculate_offset(table_id, (iinq_where.where_field));

		switch (iinq_where.bool_operator) {
			case iinq_equal:

				if (field_type == iinq_int) {
					if (NEUTRALIZE(curr, int) != int_value) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_null_terminated_string) {
					if (strncmp(char_value, (char *) curr, iinq_calculate_offset(table_id, iinq_where.where_field + 1) - iinq_calculate_offset(table_id, iinq_where.where_field)) != 0) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_char_array) {
					if (memcmp(unsigned_char_value, curr, iinq_calculate_offset(table_id, iinq_where.where_field + 1) - iinq_calculate_offset(table_id, iinq_where.where_field)) != 0) {
						return boolean_false;
					}
				}

				break;

			case iinq_not_equal:

				if (field_type == iinq_int) {
					if (NEUTRALIZE(curr, int) == int_value) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_null_terminated_string) {
					if (strncmp(char_value, (char *) curr, iinq_calculate_offset(table_id, iinq_where.where_field + 1) - iinq_calculate_offset(table_id, iinq_where.where_field)) == 0) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_char_array) {
					if (memcmp(unsigned_char_value, curr, iinq_calculate_offset(table_id, iinq_where.where_field + 1) - iinq_calculate_offset(table_id, iinq_where.where_field)) == 0) {
						return boolean_false;
					}
				}

				break;

			case iinq_less_than:

				if (field_type == iinq_int) {
					if (NEUTRALIZE(curr, int) >= int_value) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_null_terminated_string) {
					if (strncmp(char_value, (char *) curr, iinq_calculate_offset(table_id, iinq_where.where_field + 1) - iinq_calculate_offset(table_id, iinq_where.where_field)) >= 0) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_char_array) {
					if (memcmp(unsigned_char_value, curr, iinq_calculate_offset(table_id, iinq_where.where_field + 1) - iinq_calculate_offset(table_id, iinq_where.where_field)) != 0) {
						return boolean_false;
					}
				}

				break;

			case iinq_less_than_equal_to:

				if (field_type == iinq_int) {
					if (NEUTRALIZE(curr, int) > int_value) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_null_terminated_string) {
					char *value = (char *) iinq_where.field_value;

					if (strncmp(value, (char *) curr, iinq_calculate_offset(table_id, iinq_where.where_field + 1) - iinq_calculate_offset(table_id, iinq_where.where_field)) > 0) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_char_array) {
					if (memcmp(unsigned_char_value, curr, iinq_calculate_offset(table_id, iinq_where.where_field + 1) - iinq_calculate_offset(table_id, iinq_where.where_field)) != 0) {
						return boolean_false;
					}
				}

				break;

			case iinq_greater_than:

				if (field_type == iinq_int) {
					if (NEUTRALIZE(curr, int) <= int_value) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_null_terminated_string) {
					if (strncmp(char_value, (char *) curr, iinq_calculate_offset(table_id, iinq_where.where_field) - iinq_calculate_offset(table_id, iinq_where.where_field - 1)) <= 0) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_char_array) {
					if (memcmp(unsigned_char_value, curr, iinq_calculate_offset(table_id, iinq_where.where_field) - iinq_calculate_offset(table_id, iinq_where.where_field - 1)) != 0) {
						return boolean_false;
					}
				}

				break;

			case iinq_greater_than_equal_to:

				if (field_type == iinq_int) {
					if (NEUTRALIZE(curr, int) < int_value) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_null_terminated_string) {
					if (strncmp(char_value, (char *) curr, iinq_calculate_offset(table_id, iinq_where.where_field) - iinq_calculate_offset(table_id, iinq_where.where_field - 1)) < 0) {
						return boolean_false;
					}
				}
				else if (field_type == iinq_char_array) {
					if (memcmp(unsigned_char_value, curr, iinq_calculate_offset(table_id, iinq_where.where_field) - iinq_calculate_offset(table_id, iinq_where.where_field - 1)) != 0) {
						return boolean_false;
					}
				}

				break;
		}
	}

	return boolean_true;
}
