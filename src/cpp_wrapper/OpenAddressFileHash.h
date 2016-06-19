/******************************************************************************/
/**
@file		OpenAddressFileHash.h
@author		Dana Klamut
@brief		The C++ implementation of an open address file hash based
			dictionary.
*/
/******************************************************************************/

#ifndef PROJECT_OPENADDRESSFILEHASH_H
#define PROJECT_OPENADDRESSFILEHASH_H

#include "Dictionary.h"
#include "../kv_system.h"
#include "../dictionary/open_address_file_hash/open_address_file_hash_dictionary_handler.h"

class OpenAddressFileHash:public Dictionary {
public:

/**
@brief		Registers a specific open address file hash dictionary instance.

@details	Registers functions for dictionary.

@param		key_t
				The type of keys to be stored in the dictionary.
@param		key_size
				The size of keys to be stored in the dictionary.
@param	  value_size
				The size of the values to be stored in the dictionary.
@param	  dictionary_size
				The size desired for the dictionary.
 */
OpenAddressFileHash(
	key_type_t	type_key,
	int			key_size,
	int			value_size,
	int			dictionary_size
) {
	oafdict_init(&handler);

	initializeDictionary(type_key, key_size, value_size, dictionary_size);
}
};

#endif /* PROJECT_OPENADDRESSFILEHASH_H */
