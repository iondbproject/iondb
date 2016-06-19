/******************************************************************************/
/**
@file		FlatFile.h
@author		Dana Klamut
@brief		The C++ implementation of a flat file dictionary.
*/
/******************************************************************************/

#if !defined(PROJECT_FLATFILE_H)
#define PROJECT_FLATFILE_H

#include "Dictionary.h"
#include "../kv_system.h"
#include "../dictionary/flat_file/flat_file_dictionary_handler.h"

class FlatFile:public Dictionary {
public:
/**
@brief		Registers a specific flat file dictionary instance.

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
FlatFile(
	key_type_t	type_key,
	int			key_size,
	int			value_size,
	int			dictionary_size
) {
	ffdict_init(&handler);

	initializeDictionary(type_key, key_size, value_size, dictionary_size);
}
};

#endif /* PROJECT_FLATFILE_H */
