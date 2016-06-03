/******************************************************************************/
/**
@file		OpenAddressHash.h
@author		Dana Klamut
@brief      The C++ implementation of an open address hash based dictionary.
*/
/******************************************************************************/

#ifndef PROJECT_OPENADDRESSHASH_H
#define PROJECT_OPENADDRESSHASH_H


#include "Dictionary.h"
#include "../kv_system.h"
#include "../dictionary/open_address_hash/open_address_hash_dictionary_handler.h"

class OpenAddressHash : public Dictionary {

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
@param      dictionary_size
                The size desired for the dictionary.
 */
    OpenAddressHash(
            key_type_t type_key,
            int key_size,
            int value_size,
            int dictionary_size
    )
    {
        oadict_init(&handler);

        initializeDictionary(type_key, key_size, value_size, dictionary_size);
    }
};


#endif //PROJECT_OPENADDRESSHASH_H