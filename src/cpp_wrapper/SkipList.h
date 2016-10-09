/******************************************************************************/
/**
@file
@author		Dana Klamut
@brief		The C++ implementation of a skip list dictionary.
*/
/******************************************************************************/

#ifndef ION_PROJECT_SKIPLIST_H
#define ION_PROJECT_SKIPLIST_H

#include "Dictionary.h"
#include "../key_value/kv_system.h"
#include "../dictionary/skip_list/skip_list_handler.h"

template<typename K, typename V>
class SkipList:public Dictionary<K, V> {
public:
/**
@brief		Registers a specific skip list dictionary instance.

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
SkipList(
	ion_key_type_t			type_key,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size
) {
	sldict_init(&this->handler);

	this->initializeDictionary(type_key, key_size, value_size, dictionary_size);
}
};

#endif /* PROJECT_SKIPLIST_H */
