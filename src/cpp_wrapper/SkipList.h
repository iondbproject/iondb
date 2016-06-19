/******************************************************************************/
/**
@file		SkipList.h
@author		Dana Klamut
@brief		The C++ implementation of a skip list dictionary.
*/
/******************************************************************************/

#ifndef PROJECT_SKIPLIST_H
#define PROJECT_SKIPLIST_H

#include "Dictionary.h"
#include "../kv_system.h"
#include "../dictionary/skip_list/skip_list_handler.h"

class SkipList:public Dictionary {
public:
/**
@brief		Registers a specific skip list dictionary instance.

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
SkipList(
	key_type_t	type_key,
	int			key_size,
	int			value_size,
	int			dictionary_size
) {
	sldict_init(&handler);

	initializeDictionary(type_key, key_size, value_size, dictionary_size);
}
};

#endif /* PROJECT_SKIPLIST_H */
