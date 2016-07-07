/******************************************************************************/
/**
@file
@author		Dana Klamut
@brief		Interface describing how user interacts with general dictionaries
			using C++.
*/
/******************************************************************************/

#if !defined(PROJECT_CPP_DICTIONARY_H)
#define PROJECT_CPP_DICTIONARY_H

#include "../dictionary/dictionary.h"
#include "../dictionary/dictionary_types.h"
#include "../kv_system.h"

#include "Cursor.h"

template <typename K, typename V>class Dictionary {
public:

dictionary_handler_t	handler;
dictionary_t			dict;
int						size_k;
int						size_v;
ion_status_t			last_status;

~Dictionary(
) {
	this->destroy();
}

err_t
initializeDictionary(
	key_type_t	type_key,
	int			key_size,
	int			value_size,
	int			dictionary_size
) {
	err_t err = dictionary_create(&handler, &dict, 0, type_key, key_size, value_size, dictionary_size);

	size_k	= key_size;
	size_v	= value_size;

	return err;
}

/**
@brief		Insert a value into a dictionary.

@param		key
				The key that identifies @p value.
@param		value
				The value to store under @p key.
@returns	An error message describing the result of the insertion.
*/
ion_status_t
insert(
	K	key,
	V	value
) {
	ion_key_t	ion_key		= (ion_key_t) &key;
	ion_value_t ion_value	= (ion_value_t) &value;

	ion_status_t status		= dictionary_insert(&dict, ion_key, ion_value);

	this->last_status = status;

	return status;
}

V
get(
	K key
) {
	ion_key_t	ion_key = (ion_key_t) &key;
	ion_byte_t	ion_value[dict.instance->record.value_size];

	this->last_status = dictionary_get(&dict, ion_key, ion_value);

	return *((V *) ion_value);
}

/**
@brief		Delete a value given a key.

@param		key
				The key to be deleted.
@return		An error message describing the result of the deletion.
*/
ion_status_t
deleteRecord(
	K key
) {
	ion_key_t		ion_key = (ion_key_t) &key;
	ion_status_t	status	= dictionary_delete(&dict, ion_key);

	this->last_status = status;

	return status;
}

/**
@brief		Update all records with a given key.

@param		key
				The key to identify records for updating.
@param		value
				The value to update records with.
@return	 An error message describing the result of the update.
*/
ion_status_t
update(
	K	key,
	V	value
) {
	ion_key_t		ion_key		= (ion_key_t) &key;
	ion_value_t		ion_value	= (ion_value_t) &value;
	ion_status_t	status		= dictionary_update(&dict, ion_key, ion_value);

	this->last_status = status;

	return status;
}

/**
@brief	  Destroys dictionary.

@return		An error message describing the total destruction of the dictionary.
 */
err_t
destroy(
) {
	err_t err = dictionary_delete_dictionary(&dict);

	return err;
}

/**
@brief	  Opens a dictionary, given the desired config.

@param	  config_info
				The configuration of the dictionary to be opened.
@return	 An error message describing the result of of the open.
*/
err_t
open(
	ion_dictionary_config_info_t config_info
) {
	err_t err = dictionary_open(&handler, &dict, &config_info);

	return err;
}

/**
@brief	  Closes a dictionary.
*/
err_t
close(
) {
	err_t err = dictionary_close(&dict);

	return err;
}

/**
@brief	  Sets up cursor and predicate to perform a range query on a
			dictionary.

@param	  min_key
				The minimum key to be included in the query.
@param	  max_key
				The maximum key to be included in the query.
@returns	An initialized cursor for the particular query.
*/
Cursor<K, V> *
range(
	K	min_key,
	K	max_key
) {
	predicate_t predicate;
	ion_key_t	ion_min_key = (ion_key_t) &min_key;
	ion_key_t	ion_max_key = (ion_key_t) &max_key;

	dictionary_build_predicate(&predicate, predicate_range, ion_min_key, ion_max_key);
	return new Cursor<K, V>(&dict, &predicate);
}

/**
@brief	  Sets up cursor and predicate perform an equality query
			on a dictionary for a given key.

@param	  key
				The key used to determine equality.
@returns	An initialized cursor for the particular query.
*/
Cursor<K, V> *
equality(
	K key
) {
	predicate_t predicate;
	ion_key_t	ion_key = (ion_key_t) &key;

	dictionary_build_predicate(&predicate, predicate_equality, ion_key);
	return new Cursor<K, V>(&dict, &predicate);
}

/**
@brief	  Sets up cursor and predicate in order to find all records
			present in the dictionary.

@returns	An initialized cursor for the particular query.
*/
Cursor<K, V> *
allRecords(
) {
	predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);
	return new Cursor<K, V>(&dict, &predicate);
}
};

#endif /* PROJECT_CPP_DICTIONARY_H */
