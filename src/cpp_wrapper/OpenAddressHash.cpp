/******************************************************************************/
/**
@file		OpenAddressHash.cpp
@author		Dana Klamut
@brief      The C++ implementation of an open address hash based dictionary.
*/
/******************************************************************************/

#include "OpenAddressHash.h"

OpenAddressHash::OpenAddressHash(
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
    oadict_init(&handler);

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