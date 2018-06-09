/******************************************************************************/
/**
@file		iinq.c
@author		IonDB Project
@brief		This code contains definitions for iinq functions
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

#include <stdio.h>
#include "iinq.h"

ion_err_t
iinq_create_source(
	iinq_table_id		*table_id,
	ion_key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size
) {
	ion_err_t					error;
	FILE						*schema_file;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler = &handler;

	char schema_file_name[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(table_id, "inq", schema_file_name);

	error = ion_init_master_table();

	if (err_ok != error) {
		return error;
	}

	/* Load the handler. */
	ffdict_init(&handler);

	/* If the file exists, fail. */
	if (NULL != (schema_file = fopen(schema_file_name, "rb"))) {
		if (0 != fclose(schema_file)) {
			return err_file_close_error;
		}

		return err_duplicate_dictionary_error;
	}
	/* Otherwise, we are creating the dictionary for the first time. */
	else if (NULL != (schema_file = fopen(schema_file_name, "w+b"))) {
		if (0 != fseek(schema_file, 0, SEEK_SET)) {
			return err_file_bad_seek;
		}

		error = ion_master_table_create_dictionary(&handler, &dictionary, key_type, key_size, value_size, 10);

		if (err_ok != error) {
			return error;
		}

		if (1 != fwrite(&dictionary.instance->id, sizeof(dictionary.instance->id), 1, schema_file)) {
			return err_file_incomplete_read;
		}

		if (0 != fclose(schema_file)) {
			return err_file_close_error;
		}

		ion_close_dictionary(&dictionary);

		error = err_ok;
	}
	else {
		error = err_file_open_error;
	}

	ion_close_master_table();

	return error;
}

ion_err_t
iinq_open_source(
	iinq_table_id				*table_id,
	ion_dictionary_t			*dictionary,
	ion_dictionary_handler_t	*handler
) {
	ion_err_t			error;
	FILE				*schema_file;
	ion_dictionary_id_t id;

	error = ion_init_master_table();

	if (err_ok != error) {
		return error;
	}

	char schema_file_name[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(*table_id, "inq", schema_file_name);

	/* Load the handler. */
	ffdict_init(handler);

	/* If the schema file already exists. */
	if (NULL != (schema_file = fopen(schema_file_name, "rb"))) {
		if (0 != fseek(schema_file, 0, SEEK_SET)) {
			return err_file_bad_seek;
		}

		if (1 != fread(&id, sizeof(id), 1, schema_file)) {
			return err_file_incomplete_read;
		}

		error = ion_open_dictionary(handler, dictionary, id);

		if (err_ok != error) {
			return error;
		}

		if (0 != fclose(schema_file)) {
			return err_file_close_error;
		}

		error = err_ok;
	}
	else {
		error = err_file_open_error;
	}

	ion_close_master_table();

	return error;
}

ion_status_t
iinq_insert(
	iinq_table_id	*table_id,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_err_t					error;
	ion_status_t				status = ION_STATUS_INITIALIZE;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != error) {
		status = ION_STATUS_ERROR(error);
		goto RETURN;
	}

	status	= dictionary_insert(&dictionary, key, value);
	error	= ion_close_dictionary(&dictionary);

	if ((err_ok == status.error) && (err_ok != error)) {
		status.error = error;
	}

RETURN: return status;
}

ion_status_t
iinq_update(
	iinq_table_id	*table_id,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_err_t					error;
	ion_status_t				status = ION_STATUS_INITIALIZE;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != error) {
		status = ION_STATUS_ERROR(error);
		goto RETURN;
	}

	status	= dictionary_update(&dictionary, key, value);
	error	= ion_close_dictionary(&dictionary);

	if ((err_ok == status.error) && (err_ok != error)) {
		status.error = error;
	}

RETURN: return status;
}

ion_status_t
iinq_delete(
	iinq_table_id	*table_id,
	ion_key_t		key
) {
	ion_err_t					error;
	ion_status_t				status = ION_STATUS_INITIALIZE;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != error) {
		status = ION_STATUS_ERROR(error);
		goto RETURN;
	}

	status	= dictionary_delete(&dictionary, key);
	error	= ion_close_dictionary(&dictionary);

	if ((err_ok == status.error) && (err_ok != error)) {
		status.error = error;
	}

RETURN: return status;
}

ion_err_t
iinq_drop(
	iinq_table_id *table_id
) {
	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != error) {
		return error;
	}

	error = dictionary_delete_dictionary(&dictionary);

	char schema_file_name[ION_MAX_FILENAME_LENGTH];

	dictionary_get_filename(table_id, "inq", schema_file_name);

	fremove(schema_file_name);

	return error;
}

ion_comparison_t
iinq_sort_compare(
	void	*context,	/* TODO: Turn this into a ion_sort_comparator_context_t. */
	void	*a,
	void	*b
) {
#define TO_COMPARISON_RESULT(r) ((r) > 0 ? A_gt_B : ((r) < 0 ? A_lt_B : A_equ_B))

	int i;
	iinq_sort_context_t *c;
	int					result;
	void				*cur_a;
	void				*cur_b;

	result	= 0;
	c		= (iinq_sort_context_t *) context;
	cur_a	= a;
	cur_b	= b;

	if (NULL == c->parts) {
		return A_equ_B;
	}

	/* Loop through each ordering part. Stop early if possible. */
	for (i = 0; i < c->n; i++) {
		if (IINQ_ORDERTYPE_INT == c->parts->type) {
			if (1 == c->parts[i].size) {
				if (*((uint8_t *) cur_a) > *((uint8_t *) cur_b)) {
					result = 1;
				}
				else if (*((uint8_t *) cur_a) < *((uint8_t *) cur_b)) {
					result = -1;
				}
			}
			else if (2 == c->parts[i].size) {
				if (*((uint16_t *) cur_a) > *((uint16_t *) cur_b)) {
					result = 1;
				}
				else if (*((uint16_t *) cur_a) < *((uint16_t *) cur_b)) {
					result = -1;
				}
			}
			else if (4 == c->parts[i].size) {
				if (*((uint32_t *) cur_a) > *((uint32_t *) cur_b)) {
					result = 1;
				}
				else if (*((uint32_t *) cur_a) < *((uint32_t *) cur_b)) {
					result = -1;
				}
			}
			else if (8 == c->parts[i].size) {
				if (*((uint64_t *) cur_a) > *((uint64_t *) cur_b)) {
					result = 1;
				}
				else if (*((uint64_t *) cur_a) < *((uint64_t *) cur_b)) {
					result = -1;
				}
			}
		}
		else if (IINQ_ORDERTYPE_UINT == c->parts->type) {
			if (1 == c->parts[i].size) {
				if (*((int8_t *) cur_a) > *((int8_t *) cur_b)) {
					result = 1;
				}
				else if (*((int8_t *) cur_a) < *((int8_t *) cur_b)) {
					result = -1;
				}
			}
			else if (2 == c->parts[i].size) {
				if (*((int16_t *) cur_a) > *((int16_t *) cur_b)) {
					result = 1;
				}
				else if (*((int16_t *) cur_a) < *((int16_t *) cur_b)) {
					result = -1;
				}
			}
			else if (4 == c->parts[i].size) {
				if (*((int32_t *) cur_a) > *((int32_t *) cur_b)) {
					result = 1;
				}
				else if (*((int32_t *) cur_a) < *((int32_t *) cur_b)) {
					result = -1;
				}
			}
			else if (8 == c->parts[i].size) {
				if (*((int64_t *) cur_a) > *((int64_t *) cur_b)) {
					result = 1;
				}
				else if (*((int64_t *) cur_a) < *((int64_t *) cur_b)) {
					result = -1;
				}
			}
		}
		else if (IINQ_ORDERTYPE_FLOAT == c->parts->type) {
			/* TODO: Write a comparator for floats */
		}
		else if (IINQ_ORDERTYPE_OTHER == c->parts->type) {
			result = strncmp(cur_a, cur_b, c->parts[i].size);
		}

		if (result != 0) {
			result *= c->parts[i].direction;
			return TO_COMPARISON_RESULT(result);
		}

		cur_a	= ((uint8_t *) cur_a) + c->parts[i].size;
		cur_b	= ((uint8_t *) cur_b) + c->parts[i].size;
	}

	return A_equ_B;

#undef TO_COMPARISON_RESULT
}
