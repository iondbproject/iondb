#include <stdio.h>
#include "iinq.h"
#include "../dictionary/bpp_tree/bpp_tree_handler.h"

err_t
iinq_init_source(
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
	if (NULL != (schema_file = fopen(schema_file_name, "r"))) {
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
