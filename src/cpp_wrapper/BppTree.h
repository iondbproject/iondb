/******************************************************************************/
/**
@file		BppTree.h
@author		Dana Klamut
@brief      The C++ implementation of a B+ tree dictionary.
*/
/******************************************************************************/

#ifndef PROJECT_BPPTREE_H
#define PROJECT_BPPTREE_H

#include "Dictionary.h"
#include "../kv_system.h"
#include "../dictionary/bpp_tree/bpp_tree_handler.h"

template <class K>
class BppTree : public Dictionary<K> {

public:
/**
@brief		Registers a specific B+ tree dictionary instance.

@details	Registers functions for dictionary.

@param		key_t
				The type of keys to be stored in the dictionary.
@param		key_size
				The size of keys to be stored in the dictionary.
@param      value_size
                The size of the values to be stored in the dictionary.
 */
    BppTree(
            key_type_t type_key,
            int key_size,
            int value_size
    )
    {
        bpptree_init(&this->handler);

        this->initializeDictionary(type_key, key_size, value_size, 0);
    }
};


#endif //PROJECT_BPPTREE_H
