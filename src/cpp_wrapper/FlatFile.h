/******************************************************************************/
/**
@file		FlatFile.h
@author		Dana Klamut
@brief      The C++ implementation of a flat file dictionary.
*/
/******************************************************************************/

#ifndef PROJECT_FLATFILE_H
#define PROJECT_FLATFILE_H


#include "CppDictionary.h"
#include "../kv_system.h"
#include "../dictionary/flat_file/flat_file_dictionary_handler.h"

class FlatFile : public CppDictionary {

public:
/**
@brief		Registers a specific flat file dictionary instance.

@details	Registers functions for dictionary.

@param		key_t
				The type of keys to be stored in the dictionary.
@param		key_size
				The size of keys to be stored in the dictionary.
@param      value_size
                The size of the values to be stored in the dictionary.
 */
    FlatFile(
            const key_type_t &type_key,
            int key_size,
            int value_size
    );
};


#endif //PROJECT_FLATFILE_H
