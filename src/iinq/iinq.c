#include "iinq.h"

ion_err_t
iinq_create_source(
	char				*schema_file_name,
	ion_key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size
) {
	ion_err_t					error;
	FILE						*schema_file;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= ion_init_master_table();

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
	char						*schema_file_name,
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
	char		*schema_file_name,
	ion_key_t	key,
	ion_value_t value
) {
	ion_err_t					error;
	ion_status_t				status = ION_STATUS_INITIALIZE;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(schema_file_name, &dictionary, &handler);

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
	char		*schema_file_name,
	ion_key_t	key,
	ion_value_t value
) {
	ion_err_t					error;
	ion_status_t				status = ION_STATUS_INITIALIZE;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(schema_file_name, &dictionary, &handler);

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
	char		*schema_file_name,
	ion_key_t	key
) {
	ion_err_t					error;
	ion_status_t				status = ION_STATUS_INITIALIZE;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(schema_file_name, &dictionary, &handler);

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
	char *schema_file_name
) {
	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(schema_file_name, &dictionary, &handler);

	if (err_ok != error) {
		return error;
	}

	error = dictionary_delete_dictionary(&dictionary);

	fremove(schema_file_name);

	return error;
}

ion_comparison_e
iinq_sort_compare(
	void	*context,	// TODO: Turn this into a ion_sort_comparator_context_t.
	void	*a,
	void	*b
) {
#define TO_COMPARISON_RESULT(r)	((r) > 0 ? greater_than : ((r) < 0 ? less_than : equal))

	int					i;
	iinq_sort_context_t	*c;
	int 				result;
	void 				*cur_a;
	void 				*cur_b;

	result		= 0;
	c			= (iinq_sort_context_t *)context;
	cur_a		= a;
	cur_b		= b;

	if (NULL == c->parts) {
		return equal;
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
			// TODO: Write a comparator for floats
		}
		else if (IINQ_ORDERTYPE_OTHER == c->parts->type) {
			result = strncmp(cur_a, cur_b, c->parts[i].size);
		}

		if (result != 0) {
			result *= c->parts[i].direction;
			return TO_COMPARISON_RESULT(result);
		}

		cur_a = ((uint8_t *) cur_a) + c->parts[i].size;
		cur_b = ((uint8_t *) cur_b) + c->parts[i].size;
	}

	return equal;

#undef TO_COMPARISON_RESULT
}
