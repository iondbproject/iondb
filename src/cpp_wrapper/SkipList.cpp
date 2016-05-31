/******************************************************************************/
/**
@file		SkipList.cpp
@author		Dana Klamut
@brief      The C++ implementation of a skip list dictionary.
*/
/******************************************************************************/

#include "SkipList.h"

SkipList::SkipList(
        const key_type_t &type_key,
        int key_size,
        int value_size,
        int dictionary_size
) : CppDictionary(
        type_key,
        key_size,
        value_size,
        dictionary_size
)
{
    sldict_init(&handler);

    /** Where does this method get called? */
    dictionary_create(
            &handler,
            &dict,
            0,
            type_key,
            key_size,
            value_size,
            dictionary_size
    );
}