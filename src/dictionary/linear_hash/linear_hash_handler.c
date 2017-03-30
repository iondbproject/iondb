#include "linear_hash_handler.h"

void
linear_hash_dict_init(
	ion_dictionary_handler_t *handler
) {
	handler->insert				= linear_hash_dict_insert;
	handler->get				= linear_hash_dict_get;
	handler->create_dictionary	= linear_hash_create_dictionary;
	handler->remove				= linear_hash_dict_delete;
	handler->delete_dictionary	= linear_hash_delete_dictionary;
	handler->update				= linear_hash_dict_update;
	/* handler->find				= linear_hash_dict_find; */
	handler->close_dictionary	= linear_hash_close_dictionary;
	handler->open_dictionary	= linear_hash_open_dictionary;
}

ion_status_t
linear_hash_dict_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return linear_hash_insert(key, value, insert_hash_to_bucket(key, (linear_hash_table_t *) dictionary->instance), (linear_hash_table_t *) dictionary->instance);
}

ion_status_t
linear_hash_dict_get(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return linear_hash_get(key, value, (linear_hash_table_t *) dictionary->instance);
}

ion_status_t
linear_hash_dict_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
) {
	return linear_hash_update(key, value, (linear_hash_table_t *) dictionary->instance);
}

ion_err_t
linear_hash_create_dictionary(
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size,
	ion_dictionary_compare_t	compare,
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary
) {
	/* UNUSED(id); */
	int initial_size, split_threshold, records_per_bucket;

	dictionary->instance = malloc(sizeof(linear_hash_table_t));

	if (NULL == dictionary->instance) {
		return err_out_of_memory;
	}

	if (NULL == dictionary->instance) {
		return err_out_of_memory;
	}

	dictionary->instance->compare	= compare;

	initial_size					= 4;
	split_threshold					= 85;
	records_per_bucket				= 20;

	/* TODO Should we handle the possible error code returned by this? If yes, what sorts of errors does it return? */
	ion_err_t result = linear_hash_init(id, dictionary_size, key_type, key_size, value_size, initial_size, split_threshold, records_per_bucket, (linear_hash_table_t *) dictionary->instance);

	if (err_ok == result) {
		dictionary->handler = handler;
	}

	return result;
}

ion_status_t
linear_hash_dict_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
) {
	return linear_hash_delete(key, (linear_hash_table_t *) dictionary->instance);
}

ion_err_t
linear_hash_delete_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_err_t result = linear_hash_destroy((linear_hash_table_t *) dictionary->instance);

	free(dictionary->instance);
	dictionary->instance = NULL;
	return result;
}

ion_err_t
linear_hash_open_dictionary(
	ion_dictionary_handler_t		*handler,
	ion_dictionary_t				*dictionary,
	ion_dictionary_config_info_t	*config,
	ion_dictionary_compare_t		compare
) {
	return linear_hash_create_dictionary(config->id, config->type, config->key_size, config->value_size, config->dictionary_size, compare, handler, dictionary);
}

ion_err_t
linear_hash_close_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_err_t err = linear_hash_close((linear_hash_table_t *) dictionary->instance);

	if (dictionary->instance != NULL) {
		free(dictionary->instance);
		dictionary->instance = NULL;
	}

	if (err_ok != err) {
		return err;
	}

	return err_ok;
}

ion_status_t
linear_hash_dict_find(
	ion_dictionary_t *dictionary
) {
	ion_status_t status = ION_STATUS_INITIALIZE;

	UNUSED(dictionary);
	status.error = err_not_implemented;
	return status;
}
