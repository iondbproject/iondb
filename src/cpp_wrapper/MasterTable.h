/******************************************************************************/
/**
@file		MasterTable.h
@author		Dana Klamut
@brief		Interface describing how user interacts with the master table using
			C++.
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

#if !defined(PROJECT_CPP_MASTERTABLE_H)
#define PROJECT_CPP_MASTERTABLE_H

#include "../dictionary/ion_master_table.h"
#include "Dictionary.h"
#include "BppTree.h"
#include "FlatFile.h"
#include "OpenAddressFileHash.h"
#include "OpenAddressHash.h"
#include "SkipList.h"
#include "LinearHash.h"

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
@brief		Closes the master table and all dictionary instances in it.
@details	For this method to work properly, all dictionary instances must already
			be closed or must have been closed previously and are now re-opened.
@returns	An error code describing the result of the operation.
*/
ion_err_t
closeAllMasterTable(
) {
	return ion_close_all_master_table();
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
@param		key_type
				The type of key to be used with this dictionary, which
				determines the key comparison operator.
@param		k
				The type of key to be used with this dictionary.
@param		v
				The type of value to be used with this dictionary.
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
Dictionary<K, V> *
initializeDictionary(
	ion_key_type_t			key_type,
	K						k,
	V						v,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size,
	ion_dictionary_type_t	dictionary_type
) {
	UNUSED(k);
	UNUSED(v);

	Dictionary<K, V> *dictionary;

	ion_dictionary_id_t id = ion_master_table_next_id;

	switch (dictionary_type) {
		case dictionary_type_bpp_tree_t: {
			dictionary = new BppTree<K, V>(id, key_type, key_size, value_size);

			break;
		}

		case dictionary_type_flat_file_t: {
			dictionary = new FlatFile<K, V>(id, key_type, key_size, value_size, dictionary_size);

			break;
		}

		case dictionary_type_open_address_file_hash_t: {
			dictionary = new OpenAddressFileHash<K, V>(id, key_type, key_size, value_size, dictionary_size);

			break;
		}

		case dictionary_type_open_address_hash_t: {
			dictionary = new OpenAddressHash<K, V>(id, key_type, key_size, value_size, dictionary_size);

			break;
		}

		case dictionary_type_skip_list_t: {
			dictionary = new SkipList<K, V>(id, key_type, key_size, value_size, dictionary_size);

			break;
		}

		case dictionary_type_linear_hash_t: {
			dictionary = new LinearHash<K, V>(id, key_type, key_size, value_size, dictionary_size);

			break;
		}

		case dictionary_type_error_t: {
			dictionary				= new SkipList<K, V>(id, key_type, key_size, value_size, dictionary_size);
			dictionary->dict.status = ion_dictionary_status_error;
			break;
		}
	}

	if ((dictionary_type != dictionary_type_error_t) && (ion_dictionary_status_error != dictionary->dict.status)) {
		ion_err_t err = addToMasterTable(dictionary, dictionary_size);

		if (err_ok != err) {
			dictionary->dict.status = ion_dictionary_status_error;
		}
	}

	return dictionary;
}

template<typename K, typename V>
ion_err_t
deleteDictionary(
	Dictionary<K, V>		*dictionary
) {
	ion_dictionary_id_t id = dictionary->dict.instance->id;

	delete dictionary;

	return ion_delete_from_master_table(id);
}
};

#endif /* PROJECT_CPP_MASTERTABLE_H */
