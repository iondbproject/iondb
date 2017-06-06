/******************************************************************************/
/**
@file
@author		Eric Huang, Graeme Douglas, Scott Fazackerley, Wade Penson, Dana Klamut
@brief		Master table handling.
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

#include "ion_master_table.h"
#include "bpp_tree/bpp_tree_handler.h"
#include "flat_file/flat_file_dictionary_handler.h"
#include "open_address_file_hash/open_address_file_hash_dictionary_handler.h"
#include "open_address_hash/open_address_hash_dictionary_handler.h"
#include "skip_list/skip_list_handler.h"

FILE				*ion_master_table_file		= NULL;
ion_dictionary_id_t ion_master_table_next_id	= 1;

#define ION_MASTER_TABLE_CALCULATE_POS	-1
#define ION_MASTER_TABLE_WRITE_FROM_END -2
#define ION_MASTER_TABLE_RECORD_SIZE(cp) (sizeof((cp)->id) + sizeof((cp)->use_type) + sizeof((cp)->type) + sizeof((cp)->key_size) + sizeof((cp)->value_size) + sizeof((cp)->dictionary_size) + sizeof((cp)->dictionary_type))

/**
@brief		Write a record to the master table.
@details	Automatically, this call will reposition the file position
			back to where it was once the call is complete.
@param[in]	config
				A pointer to a previously allocated config object to write from.
@param[in]	where
				An integral value representing where to write to in the file.
				This file offset is byte-aligned, not record aligned, in general.

				Two special flags can be passed in here:
					- @c ION_MASTER_TABLE_CALCULATE_POS
						Calculate the position based on the passed-in config id.
					- @c ION_MASTER_TABLE_WRITE_FROM_END
						Write the record at the end of the file.
@returns	An error code describing the result of the call.
*/
ion_err_t
ion_master_table_write(
	ion_dictionary_config_info_t	*config,
	long							where
) {
	long old_pos = ftell(ion_master_table_file);

	if (ION_MASTER_TABLE_CALCULATE_POS == where) {
		where = (int) (config->id * ION_MASTER_TABLE_RECORD_SIZE(config));
	}

	if (ION_MASTER_TABLE_CALCULATE_POS > where) {
		if (0 != fseek(ion_master_table_file, 0, SEEK_END)) {
			return err_file_bad_seek;
		}
	}
	else if (0 != fseek(ion_master_table_file, where, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (1 != fwrite(&(config->id), sizeof(config->id), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->use_type), sizeof(config->use_type), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->type), sizeof(config->type), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->key_size), sizeof(config->key_size), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->value_size), sizeof(config->value_size), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->dictionary_size), sizeof(config->dictionary_size), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fwrite(&(config->dictionary_type), sizeof(config->dictionary_type), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (0 != fseek(ion_master_table_file, old_pos, SEEK_SET)) {
		return err_file_bad_seek;
	}

	return err_ok;
}

/**
@brief		Read a record to the master table.
@details	Automatically, this call will reposition the file position
			back to where it was once the call is complete.
@param[out]	config
				A pointer to a previously allocated config object to write to.
@param[in]	where
				An integral value representing where to read from in the file.
				This file offset is byte-aligned, not record aligned, in general.

				One special flag can be passed in here:
					- @c ION_MASTER_TABLE_CALCULATE_POS
						Calculate the position based on the passed-in config id.
@returns	An error code describing the result of the call.
*/
ion_err_t
ion_master_table_read(
	ion_dictionary_config_info_t	*config,
	long							where
) {
	long old_pos = ftell(ion_master_table_file);

	if (ION_MASTER_TABLE_CALCULATE_POS == where) {
		where = (int) (config->id * ION_MASTER_TABLE_RECORD_SIZE(config));
	}

	if (0 != fseek(ion_master_table_file, where, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (1 != fread(&(config->id), sizeof(config->id), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fread(&(config->use_type), sizeof(config->use_type), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fread(&(config->type), sizeof(config->type), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fread(&(config->key_size), sizeof(config->key_size), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fread(&(config->value_size), sizeof(config->value_size), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fread(&(config->dictionary_size), sizeof(config->dictionary_size), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (1 != fread(&(config->dictionary_type), sizeof(config->dictionary_type), 1, ion_master_table_file)) {
		return err_file_write_error;
	}

	if (0 != fseek(ion_master_table_file, old_pos, SEEK_SET)) {
		return err_file_bad_seek;
	}

	if (0 == config->id) {
		return err_item_not_found;
	}

	return err_ok;
}

/* Returns the next dictionary ID, then increments. */
ion_err_t
ion_master_table_get_next_id(
	ion_dictionary_id_t *id
) {
	ion_err_t error								= err_ok;

	/* Flush master row. This writes the next ID to be used, so add 1. */
	ion_dictionary_config_info_t master_config	= { .id = ion_master_table_next_id + 1 };

	error = ion_master_table_write(&master_config, 0);

	if (err_ok != error) {
		return error;
	}

	*id = ion_master_table_next_id++;

	return err_ok;
}

ion_err_t
ion_init_master_table(
	void
) {
	ion_err_t error = err_ok;

	/* If it's already open, then we don't do anything. */
	if (NULL != ion_master_table_file) {
		return err_ok;
	}

	ion_master_table_file = fopen(ION_MASTER_TABLE_FILENAME, "r+b");

	/* File may not exist. */
	if (NULL == ion_master_table_file) {
		ion_master_table_file = fopen(ION_MASTER_TABLE_FILENAME, "w+b");

		if (NULL == ion_master_table_file) {
			return err_file_open_error;
		}

		/* Clean fresh file was opened. */
		/* Write master row. */
		ion_dictionary_config_info_t master_config = { .id = ion_master_table_next_id };

		if (err_ok != (error = ion_master_table_write(&master_config, 0))) {
			return error;
		}
	}
	else {
		/* Here we read an existing file. */

		/* Find existing ID count. */
		ion_dictionary_config_info_t master_config;

		if (ion_master_table_read(&master_config, 0)) {
			return err_file_read_error;
		}

		ion_master_table_next_id = master_config.id;
	}

	return err_ok;
}

ion_err_t
ion_close_master_table(
	void
) {
	ion_err_t					err;
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	ion_dictionary_id_t id = ion_master_table_next_id;

	if (NULL != ion_master_table_file) {
		id--;

		while (id > 0) {
			err = ion_open_dictionary(&handler, &dict, id);

			if (err_ok != err) {
				/* Dictionary not found, continue search. */
				if (err_dictionary_initialization_failed != err) {
					return err;
				}
			}
			else {
				err = ion_close_dictionary(&dict);

				if (err_ok != err) {
					return err;
				}
			}

			id--;
		}

		if (0 != fclose(ion_master_table_file)) {
			return err_file_close_error;
		}
	}

	ion_master_table_file = NULL;

	return err_ok;
}

ion_err_t
ion_delete_master_table(
	void
) {
	if (0 != fremove(ION_MASTER_TABLE_FILENAME)) {
		return err_file_delete_error;
	}

	return err_ok;
}

/**
@brief		Adds the given dictionary to the master table.
@param		dictionary
				A pointer to the dictionary object to add to the master table.
@param		dictionary_size
				The implementation specific size parameter used when
				creating the dictionary. This parameter must be passed
				to this function by @ref ion_master_table_create_dictionary,
				since not all implementations track the dictionary size.
*/
ion_err_t
ion_add_to_master_table(
	ion_dictionary_t		*dictionary,
	ion_dictionary_size_t	dictionary_size
) {
	ion_dictionary_config_info_t config = {
		.id = dictionary->instance->id, .use_type = 0, .type = dictionary->instance->key_type, .key_size = dictionary->instance->record.key_size, .value_size = dictionary->instance->record.value_size, .dictionary_size = dictionary_size, .dictionary_type = dictionary->instance->type
	};

	return ion_master_table_write(&config, ION_MASTER_TABLE_WRITE_FROM_END);
}

ion_err_t
ion_master_table_create_dictionary(
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size
) {
	ion_err_t			err;
	ion_dictionary_id_t id;

	err = ion_master_table_get_next_id(&id);

	if (err_ok != err) {
		return err;
	}

	err = dictionary_create(handler, dictionary, id, key_type, key_size, value_size, dictionary_size);

	if (err_ok != err) {
		return err;
	}

	err = ion_add_to_master_table(dictionary, dictionary_size);

	return err;
}

ion_err_t
ion_lookup_in_master_table(
	ion_dictionary_id_t				id,
	ion_dictionary_config_info_t	*config
) {
	ion_err_t error = err_ok;

	config->id	= id;
	error		= ion_master_table_read(config, ION_MASTER_TABLE_CALCULATE_POS);

	if (err_ok != error) {
		return error;
	}

	return err_ok;
}

ion_err_t
ion_find_by_use_master_table(
	ion_dictionary_config_info_t	*config,
	ion_dict_use_t					use_type,
	char							whence
) {
	ion_dictionary_id_t				id;
	ion_dictionary_config_info_t	tconfig;
	ion_err_t						error;

	tconfig.id	= 0;

	id			= 1;

	if (ION_MASTER_TABLE_FIND_LAST == whence) {
		id = ion_master_table_next_id - 1;
	}

	/* Loop through all items. */
	for (; id < ion_master_table_next_id && id > 0; id += whence) {
		error = ion_lookup_in_master_table(id, &tconfig);

		if (err_item_not_found == error) {
			continue;
		}

		if (err_ok != error) {
			return error;
		}

		/* If this config has the right type, set the output pointer. */
		if (tconfig.use_type == use_type) {
			*config = tconfig;

			return err_ok;
		}
	}

	return err_item_not_found;
}

ion_err_t
ion_delete_from_master_table(
	ion_dictionary_id_t id
) {
	ion_dictionary_config_info_t	blank	= { 0 };
	long							where	= (id * ION_MASTER_TABLE_RECORD_SIZE(&blank));

	return ion_master_table_write(&blank, where);
}

ion_dictionary_type_t
ion_get_dictionary_type(
	ion_dictionary_id_t id
) {
	ion_err_t						err;
	ion_dictionary_config_info_t	config;

	err = ion_lookup_in_master_table(id, &config);

	if (err_ok != err) {
		return dictionary_type_error_t;
	}

	return config.dictionary_type;
}

ion_err_t
ion_open_dictionary(
	ion_dictionary_handler_t	*handler,		/* Passed in empty, to be set. */
	ion_dictionary_t			*dictionary,	/* Passed in empty, to be set. */
	ion_dictionary_id_t			id
) {
	ion_err_t						err;
	ion_dictionary_config_info_t	config;

	err = ion_lookup_in_master_table(id, &config);

	/* Lookup for id failed. */
	if (err_ok != err) {
		return err_dictionary_initialization_failed;
	}

	ion_switch_handler(config.dictionary_type, handler);

	err = dictionary_open(handler, dictionary, &config);
	return err;
}

ion_err_t
ion_close_dictionary(
	ion_dictionary_t *dictionary
) {
	ion_err_t err;

	err = dictionary_close(dictionary);

	return err;
}

ion_err_t
ion_delete_dictionary(
	ion_dictionary_t	*dictionary,
	ion_dictionary_id_t id
) {
	ion_err_t				err;
	ion_dictionary_type_t	type;

	if (ion_dictionary_status_closed != dictionary->status) {
		id	= dictionary->instance->id;

		err = dictionary_delete_dictionary(dictionary);

		if (err_ok != err) {
			return err_dictionary_destruction_error;
		}

		err = ion_delete_from_master_table(id);
	}
	else {
		type = ion_get_dictionary_type(id);

		if (dictionary_type_error_t == type) {
			return err_dictionary_destruction_error;
		}

		ion_dictionary_handler_t handler;

		ion_switch_handler(type, &handler);

		err = dictionary_destroy_dictionary(&handler, id);

		if (err_ok != err) {
			return err;
		}

		err = ion_delete_from_master_table(id);
	}

	return err;
}

ion_err_t
ion_switch_handler(
	ion_dictionary_type_t		type,
	ion_dictionary_handler_t	*handler
) {
	switch (type) {
		case dictionary_type_bpp_tree_t: {
			bpptree_init(handler);
			break;
		}

		case dictionary_type_flat_file_t: {
			ffdict_init(handler);
			break;
		}

		case dictionary_type_open_address_file_hash_t: {
			oafdict_init(handler);
			break;
		}

		case dictionary_type_open_address_hash_t: {
			oadict_init(handler);
			break;
		}

		case dictionary_type_skip_list_t: {
			sldict_init(handler);
			break;
		}

		case dictionary_type_error_t: {
			return err_dictionary_initialization_failed;
		}
	}

	return err_ok;
}
