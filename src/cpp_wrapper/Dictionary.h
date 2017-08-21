/******************************************************************************/
/**
@file		Dictionary.h
@author		Dana Klamut, Eric Huang
@brief		Interface describing how user interacts with general dictionaries
			using C++.
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

#if !defined(PROJECT_CPP_DICTIONARY_H)
#define PROJECT_CPP_DICTIONARY_H

#include "../dictionary/dictionary.h"
#include "../dictionary/dictionary_types.h"
#include "../key_value/kv_system.h"

#include "Cursor.h"

template<typename K, typename V>
class Dictionary {
public:

<<<<<<< HEAD
ion_dictionary_handler_t	handler;
ion_dictionary_t			dict;
ion_key_type_t				key_type;
ion_key_size_t				key_size;
ion_value_size_t			value_size;
ion_dictionary_size_t		dict_size;
ion_status_t				last_status;
=======
ion_dictionary_handler_t handler;
ion_dictionary_t dict;
ion_key_size_t size_k;
ion_value_size_t size_v;
ion_dictionary_size_t dict_size;
ion_status_t last_status;
>>>>>>> origin/iinq-rewrite

~Dictionary(
) {
	this->deleteDictionary();
}

/**
@brief		Creates a dictionary with a specific identifier (for use through
			the master table).
@param		dict_id
				A unique identifier important for use of the dictionary through
				the master table. If the dictionary is being created without
				the master table, this identifier can be 0.
@param		k_type
				The type of key to be used with this dictionary, which
				determines the key comparison operator.
@param		k_size
				The size of the key type to be used with this dictionary.
@param		v_size
				The size of the value type to be used with this dictionary.
@param		dictionary_size
				The dictionary implementation specific dictionary size
				parameter.
@returns	An error code describing the result of the operation.
*/
ion_err_t
initializeDictionary(
	ion_dictionary_id_t		dict_id,
	ion_key_type_t			k_type,
	ion_key_size_t			k_size,
	ion_value_size_t		v_size,
	ion_dictionary_size_t	dictionary_size
) {
	key_type	= k_type;
	key_size	= k_size;
	value_size	= v_size;
	dict_size	= dictionary_size;

	ion_err_t err = dictionary_create(&handler, &dict, dict_id, k_type, k_size, v_size, dictionary_size);

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
	ion_key_t	ion_key		= &key;
	ion_value_t ion_value	= &value;

	ion_status_t status		= dictionary_insert(&dict, ion_key, ion_value);

	this->last_status = status;

	return status;
}

/**
@brief		Retrieve a value given a key.

@param		key
				The key to retrieve the value for.
@return		value
				A pointer to the value byte array to copy data into.
*/
V
get(
	K key
) {
	ion_key_t	ion_key = &key;
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
	ion_key_t		ion_key = &key;
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
	ion_key_t		ion_key		= &key;
	ion_value_t		ion_value	= &value;
	ion_status_t	status		= dictionary_update(&dict, ion_key, ion_value);

	this->last_status = status;

	return status;
}

/**
@brief	  Deletes dictionary.

@return		An error message describing the total destruction of the dictionary.
*/
ion_err_t
deleteDictionary(
) {
	ion_err_t err = dictionary_delete_dictionary(&dict);

	return err;
}

/**
@brief	  Destroys dictionary
@param	  id
				The identifier identifying the dictionary to destroy.
@return		The status of the total destruction of the dictionary.
*/
ion_err_t
destroyDictionary(
	ion_dictionary_id_t id
) {
	ion_err_t error = dictionary_destroy_dictionary(&handler, id);

	return error;
}

/**
@brief	  Opens a dictionary, given the desired config.

@param	  config_info
				The configuration of the dictionary to be opened.
@return	 An error message describing the result of of the open.
*/
ion_err_t
open(
	ion_dictionary_config_info_t config_info
) {
	ion_err_t err = dictionary_open(&handler, &dict, &config_info);

	return err;
}

/**
@brief	  Closes a dictionary.
*/
ion_err_t
close(
) {
	ion_err_t err = dictionary_close(&dict);

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
	ion_predicate_t predicate;
	ion_key_t		ion_min_key = &min_key;
	ion_key_t		ion_max_key = &max_key;

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
	ion_predicate_t predicate;
	ion_key_t		ion_key = &key;

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
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);
	return new Cursor<K, V>(&dict, &predicate);
}
};

/* Comment to trigger commit and therefore pc-build */

#endif /* PROJECT_CPP_DICTIONARY_H */
