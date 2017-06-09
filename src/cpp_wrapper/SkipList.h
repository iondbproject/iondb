/******************************************************************************/
/**
@file
@author		Dana Klamut
@brief		The C++ implementation of a skip list dictionary.
*/
/******************************************************************************/

#ifndef PROJECT_SKIPLIST_H
#define PROJECT_SKIPLIST_H

#include "Dictionary.h"
#include "../key_value/kv_system.h"
#include "../dictionary/skip_list/skip_list_handler.h"

template<typename K, typename V>
class SkipList:public Dictionary<K, V> {
public:
/**
@brief		Registers a specific skip list dictionary instance.

@details	Registers functions for dictionary.

@param		key_type
				The type of keys to be stored in the dictionary.
@param		key_size
				The size of keys to be stored in the dictionary.
@param	  value_size
				The size of the values to be stored in the dictionary.
@param	  dictionary_size
				The size desired for the dictionary.
*/
SkipList(
	ion_dictionary_id_t		id,
	ion_key_type_t			key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size
) {
	this->dict_type = dictionary_type_skip_list_t;

	sldict_init(&this->handler);

	this->initializeDictionary(id, key_type, key_size, value_size, dictionary_size);
}
};

#endif /* PROJECT_SKIPLIST_H */
