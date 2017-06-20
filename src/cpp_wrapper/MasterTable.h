/******************************************************************************/
/**
@file
@author		Dana Klamut
@brief		Interface describing how user interacts with the master table using
			C++.
*/
/******************************************************************************/

#if !defined(PROJECT_CPP_MASTERTABLE_H)
#define PROJECT_CPP_MASTERTABLE_H

#include "../dictionary/ion_master_table.h"
#include "../dictionary/ion_master_table.c"
#include "Dictionary.h"
#include "BppTree.h"
#include "FlatFile.h"
#include "OpenAddressFileHash.h"
#include "OpenAddressHash.h"
#include "SkipList.h"

class MasterTable {
public:

ion_dictionary_handler_t handler;

MasterTable(
) {
	this->initializeMasterTable();
}

~MasterTable(
) {
	this->deleteMasterTable();
}

/**
@brief	  Opens the master table.
@details	Can be safely called multiple times without closing.
@returns	An error code describing the result of the operation.
*/
ion_err_t
initializeMasterTable(
) {
	return ion_init_master_table();
}

/**
@brief		Closes the master table.
@returns	An error code describing the result of the operation.
*/
ion_err_t
closeMasterTable(
) {
	return ion_close_master_table();
}

/**
@brief		Deletes the master table.
@returns	An error code describing the result of the operation.
*/
ion_err_t
deleteMasterTable(
) {
	return ion_delete_master_table();
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
template<typename K, typename V>
ion_err_t
addToMasterTable(
	Dictionary<K, V>		*dictionary,
	ion_dictionary_size_t dictionary_size
) {
	ion_dictionary_id_t id;

	ion_err_t err = ion_master_table_get_next_id(&id);

	if (err_ok != err) {
		return err;
	}

	dictionary->dict.instance->id = id;

	ion_dictionary_config_info_t config = {
		.id = dictionary->dict.instance->id, .use_type = 0, .type = dictionary->dict.instance->key_type, .key_size = dictionary->dict.instance->record.key_size, .value_size = dictionary->dict.instance->record.value_size, .dictionary_size = dictionary_size, .dictionary_type = dictionary->dict.instance->type
	};

	return ion_master_table_write(&config, ION_MASTER_TABLE_WRITE_FROM_END);
}

/**
@brief		Returns the next dictionary ID.
@returns	The next ID to be used by the master table.
*/
ion_dictionary_id_t
getNextID(
) {
	return ion_master_table_next_id;
}

/**
@brief		Creates a dictionary through use of the master table.
@param		dictionary
				A pointer to an allocated dictionary object, which will be
				written into when opened.
@param		key_type
				The type of key to be used with this dictionary, which
				determines the key comparison operator.
@param		key_size
				The size of the key type to be used with this dictionary.
@param		value_size
				The size of the value type to be used with this dictionary.
@param		dictionary_size
				The dictionary implementation specific dictionary size
				parameter.
@param		dictionary_type
				The kind of dictionary instance to be created.
@returns	An error code describing the result of the operation.
*/
template<typename K, typename V>
ion_err_t
createDictionary(
	Dictionary<K, V>		 *dictionary,
	ion_key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
) {
	ion_dictionary_id_t id;

	ion_err_t err = ion_master_table_get_next_id(&id);

	if (err_ok != err) {
		return err;
	}

	err = initializeDictionary(dictionary, id, key_type, key_size, value_size, dictionary_size, dictionary_type);

	if (err_ok != err) {
		return err;
	}

	dictionary->dict.open_status	= boolean_true;

	err								= addToMasterTable(dictionary, dictionary_size);

	return err;
}

/**
@brief		Retrieves the type of dictionary stored under a particular id in the
			master table.
@param		type
				The type of dictionary instance to initialize the handler to.
@returns	An error code describing the result of the operation.
*/
ion_err_t
initializeHandler(
	ion_dictionary_type_t type
) {
	return ion_switch_handler(type, &handler);
}

/**
@brief		Looks up the config of the given id.
@param		id
				The identifier identifying the dictionary metadata in the
				master table which is to be looked up.
@param		config
				A pointer to an already allocated configuration object
				that will be read into from the master table.
@returns	An error code describing the result of the operation.
*/
ion_err_t
lookupMasterTable(
	ion_dictionary_id_t				id,
	ion_dictionary_config_info_t	*config
) {
	return ion_lookup_in_master_table(id, config);
}

/**
@brief		Find first or last dictionary in master table with a given use.
@param		config
				A pointer to an already allocated configuration object
				that will be used to open the found dictionary.
@param		use_type
				The integral usage type for the dictionary. This is
				user defined.
@param		whence
				Whether to find the first or the last dictionary
				having @p use_type. This can take exactly two values,
				@ref ION_MASTER_TABLE_FIND_FIRST or
				@ref ION_MASTER_TABLE_FIND_LAST.
@returns	@c err_ok if a dictionary having usage type @p use_type
			is found, otherwise an error code properly describing
			the situation and outcome.
*/
ion_err_t
findByUse(
	ion_dictionary_config_info_t	*config,
	ion_dict_use_t					use_type,
	char							whence
) {
	return ion_find_by_use_master_table(config, use_type, whence);
}

/**
@brief		Deletes a dictionary from the master table.
@returns	An error code describing the result of the operation.
*/
ion_err_t
deleteFromMasterTable(
	ion_dictionary_id_t id
) {
	return ion_delete_from_master_table(id);
}

/**
@brief		Deletes a dictionary instance and erases it from the master table.
@param		dictionary
				A pointer to an allocated dictionary object, which will be
				deleted.
@param		id
				The identifier identifying the dictionary metadata in the
				master table.
@returns	An error code describing the result of the operation.
*/
template<typename K, typename V>
ion_err_t
deleteDictionary(
	Dictionary<K, V>	*dictionary,
	ion_dictionary_id_t id
) {
	ion_err_t				err;
	ion_dictionary_type_t	type;

	if (ion_dictionary_status_closed != dictionary->dict.status) {
		id	= dictionary->dict.instance->id;

		err = dictionary->deleteDictionary();

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

		if (err_not_implemented == err) {
			err = ffdict_destroy_dictionary(id);
		}

		if (err_ok != err) {
			return err;
		}

		err = ion_delete_from_master_table(id);
	}

	return err;
}

/**
@brief		Finds the target dictionary and opens it.
@param		dictionary
				A pointer to an allocated dictionary object, which will be
				opened.
@param		id
				The identifier identifying the dictionary metadata in the
				master table.
@returns	An error code describing the result of the operation.
*/
template<typename K, typename V>
ion_err_t
openDictionary(
	Dictionary<K, V>	*dictionary,
	ion_dictionary_id_t id
) {
	ion_err_t						err;
	ion_dictionary_config_info_t	config;

	err = ion_lookup_in_master_table(id, &config);

	/* Lookup for id failed. */
	if (err_ok != err) {
		return err_uninitialized;
	}

	err = ion_switch_handler(config.dictionary_type, &handler);

	if (err_ok != err) {
		return err;
	}

	dictionary->dict.handler	= &handler;

	err							= dictionary->open(config);

	return err;
}

/**
@brief		Closes a given dictionary.
@param		dictionary
				A pointer to an allocated dictionary object, which will be
				closed.
@returns	An error code describing the result of the operation.
*/
template<typename K, typename V>
ion_err_t
closeDictionary(
	Dictionary<K, V> *dictionary
) {
	ion_err_t err = dictionary->close();

	return err;
}

/**
@brief		Creates a dictionary of a specified implementation.
@details	This method is not currently used for creation of a C++ dictionary.
			Modifications to be made in the future for full use.
@param		dictionary
				A pointer to an allocated dictionary object to be initialized.
@param		id
				A unique identifier important for use of the dictionary through
				the master table. If the dictionary is being created without
				the master table, this identifier can be 0.
@param		key_type
				The type of key to be used with this dictionary, which
				determines the key comparison operator.
@param		key_size
				The size of the key type to be used with this dictionary.
@param		value_size
				The size of the value type to be used with this dictionary.
@param		dictionary_size
				The dictionary implementation specific dictionary size
				parameter.
@param		dictionary_type
				The type of dictionary to be created.
@returns	An error code describing the result of the operation.
*/
template<typename K, typename V>
ion_err_t
initializeDictionary(
	Dictionary<K, V>		*dictionary,
	ion_dictionary_id_t id,
	ion_key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
) {
	switch (dictionary_type) {
		case dictionary_type_bpp_tree_t: {
			dictionary = new BppTree<K, V>(id, key_type, key_size, value_size);

			if (err_ok != dictionary->last_status.error) {
				return dictionary->last_status.error;
			}

			break;
		}

		case dictionary_type_flat_file_t: {
			dictionary = new FlatFile<K, V>(id, key_type, key_size, value_size, dictionary_size);

			if (err_ok != dictionary->last_status.error) {
				return dictionary->last_status.error;
			}

			break;
		}

		case dictionary_type_open_address_file_hash_t: {
			dictionary = new OpenAddressFileHash<K, V>(id, key_type, key_size, value_size, dictionary_size);

			if (err_ok != dictionary->last_status.error) {
				return dictionary->last_status.error;
			}

			break;
		}

		case dictionary_type_open_address_hash_t: {
			dictionary = new OpenAddressHash<K, V>(id, key_type, key_size, value_size, dictionary_size);

			if (err_ok != dictionary->last_status.error) {
				return dictionary->last_status.error;
			}

			break;
		}

		case dictionary_type_skip_list_t: {
			dictionary = new SkipList<K, V>(id, key_type, key_size, value_size, dictionary_size);

			if (err_ok != dictionary->last_status.error) {
				return dictionary->last_status.error;
			}

			break;
		}

		case dictionary_type_error_t: {
			return err_uninitialized;
		}
	}

	UNUSED(dictionary);
	return err_ok;
}
};

#endif /* PROJECT_CPP_MASTERTABLE_H */
