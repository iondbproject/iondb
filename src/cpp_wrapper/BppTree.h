/******************************************************************************/
/**
@file
@author		Dana Klamut
@brief		The C++ implementation of a B+ tree dictionary.
*/
/******************************************************************************/

#if !defined(PROJECT_BPPTREE_H)
#define PROJECT_BPPTREE_H

#include "Dictionary.h"
#include "../key_value/kv_system.h"
#include "../dictionary/bpp_tree/bpp_tree_handler.h"

template<typename K, typename V>
class BppTree:public Dictionary<K, V> {
public:
/**
@brief		Registers a specific B+ tree dictionary instance.

@details	Registers functions for dictionary.

@param		key_type
				The type of keys to be stored in the dictionary.
@param		key_size
				The size of keys to be stored in the dictionary.
@param		value_size
				The size of the values to be stored in the dictionary.
*/
BppTree(
	ion_dictionary_id_t id,
	ion_key_type_t		key_type,
	int					key_size,
	int					value_size
) {
	this->dict_type = dictionary_type_bpp_tree_t;

	bpptree_init(&this->handler);

	this->initializeDictionary(id, key_type, key_size, value_size, 0);
}
};

#endif /* PROJECT_BPPTREE_H */
