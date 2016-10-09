/******************************************************************************/
/**
@file
@author		Dana Klamut
@brief		The C++ implementation of an open address hash based dictionary.
*/
/******************************************************************************/

#ifndef ION_PROJECT_OPENADDRESSHASH_H
#define ION_PROJECT_OPENADDRESSHASH_H

#include "Dictionary.h"
#include "../key_value/kv_system.h"
#include "../dictionary/open_address_hash/open_address_hash_dictionary_handler.h"

template<typename K, typename V>
class OpenAddressHash:public Dictionary<K, V> {
public:
/**
@brief		Registers a specific open address hash dictionary instance.

@details	Registers functions for dictionary.

@param		type_key
				The type of keys to be stored in the dictionary.
@param		key_size
				The size of keys to be stored in the dictionary.
@param	  value_size
				The size of the values to be stored in the dictionary.
@param	  dictionary_size
				The size desired for the dictionary.
*/
OpenAddressHash(
	ion_key_type_t			type_key,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size
) {
	oadict_init(&this->handler);

	this->initializeDictionary(type_key, key_size, value_size, dictionary_size);
}
};

#endif /* PROJECT_OPENADDRESSHASH_H */
