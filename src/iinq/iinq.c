#include <stdio.h>
#include "iinq.h"
#include "../dictionary/bpp_tree/bpp_tree_handler.h"

err_t
iinq_create_source(
	char					*schema_file_name,
	key_type_t				key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size
) {
	err_t					error;
	FILE					*schema_file;
	dictionary_t			dictionary;
	dictionary_handler_t	handler;

	dictionary.handler		= &handler;

	error = ion_init_master_table();

	if (err_ok != error) {
		return error;
	}

	/* Load the handler. */
	bpptree_init(&handler);

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

		error = ion_master_table_create_dictionary(&handler, &dictionary, key_type, key_size, value_size, -1);

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

err_t
iinq_open_source(
	char					*schema_file_name,
	dictionary_t			*dictionary,
	dictionary_handler_t	*handler
) {
	err_t				error;
	FILE				*schema_file;
	ion_dictionary_id_t id;

	error = ion_init_master_table();

	if (err_ok != error) {
		return error;
	}

	/* Load the handler. */
	bpptree_init(handler);

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
	char 		*schema_file_name,
	ion_key_t	key,
	ion_value_t value
) {
	err_t					error;
	ion_status_t			status		= ION_STATUS_INITIALIZE;
	dictionary_t			dictionary;
	dictionary_handler_t	handler;

	dictionary.handler		= &handler;

	error					= iinq_open_source(schema_file_name, &dictionary, &handler);
	if (err_ok != error) {
		status				= ION_STATUS_ERROR(error);
		goto RETURN;
	}

	status					= dictionary_insert(&dictionary, key, value);
	error					= ion_close_dictionary(&dictionary);
	if (err_ok == status.error && err_ok != error) {
		status.error		= error;
	}

	RETURN:
	return status;
}

ion_status_t
iinq_update(
	char 		*schema_file_name,
	ion_key_t	key,
	ion_value_t value
) {
	err_t					error;
	ion_status_t			status		= ION_STATUS_INITIALIZE;
	dictionary_t			dictionary;
	dictionary_handler_t	handler;

	dictionary.handler		= &handler;

	error					= iinq_open_source(schema_file_name, &dictionary, &handler);
	if (err_ok != error) {
		status				= ION_STATUS_ERROR(error);
		goto RETURN;
	}

	status					= dictionary_update(&dictionary, key, value);
	error					= ion_close_dictionary(&dictionary);
	if (err_ok == status.error && err_ok != error) {
		status.error		= error;
	}

	RETURN:
	return status;
}

ion_status_t
iinq_delete(
	char 		*schema_file_name,
	ion_key_t	key
) {
	err_t					error;
	ion_status_t			status		= ION_STATUS_INITIALIZE;
	dictionary_t			dictionary;
	dictionary_handler_t	handler;

	dictionary.handler		= &handler;

	error					= iinq_open_source(schema_file_name, &dictionary, &handler);
	if (err_ok != error) {
		status				= ION_STATUS_ERROR(error);
		goto RETURN;
	}

	status					= dictionary_delete(&dictionary, key);
	error					= ion_close_dictionary(&dictionary);
	if (err_ok == status.error && err_ok != error) {
		status.error		= error;
	}

	RETURN:
	return status;
}

err_t
iinq_drop(
	char *schema_file_name
) {
	err_t					error;
	dictionary_t			dictionary;
	dictionary_handler_t	handler;

	dictionary.handler		= &handler;

	error					= iinq_open_source(schema_file_name, &dictionary, &handler);
	if (err_ok != error) {
		return error;
	}

	error					= dictionary_delete_dictionary(&dictionary);

	fremove(schema_file_name);

	return error;
}
