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

class MasterTable {
public:

ion_dictionary_handler_t		handler;
ion_dictionary_t				dict;
ion_dictionary_config_info_t	config;

~MasterTable(
) {
	deleteMasterTable();
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
ion_err_t
createDictionary(
	ion_dictionary_t		*dictionary,
	ion_key_type_t			key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size,
	ion_dictionary_type_t	dictionary_type
) {
	ion_err_t err = initializeHandler(dictionary_type);

	if (err_ok != err) {
		return err;
	}

	return ion_master_table_create_dictionary(&handler, dictionary, key_type, key_size, value_size, dictionary_size);
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
@returns	An error code describing the result of the operation.
*/
ion_err_t
lookupMasterTable(
	ion_dictionary_id_t id
) {
	return ion_lookup_in_master_table(id, &config);
}

/**
@brief		Find first or last dictionary in master table with a given use.
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
	ion_dict_use_t	use_type,
	char			whence
) {
	return ion_find_by_use_master_table(&config, use_type, whence);
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
@returns	An error code describing the result of the operation.
*/
ion_err_t
deleteDictionary(
	ion_dictionary_id_t id
) {
	dict.status		= ion_dictionary_status_closed;
	dict.handler	= &handler;

	return ion_delete_dictionary(&dict, id);
}

/**
@brief		Finds the target dictionary and opens it.
@param		id
				The identifier identifying the dictionary metadata in the
				master table.
@returns	An error code describing the result of the operation.
*/
ion_err_t
openDictionary(
	ion_dictionary_id_t id
) {
	return ion_open_dictionary(&handler, &dict, id);
}

/**
@brief		Closes a given dictionary.
@returns	An error code describing the result of the operation.
*/
ion_err_t
closeDictionary(
) {
	return ion_close_dictionary(&dict);
}
};

#endif /* PROJECT_CPP_MASTERTABLE_H */
