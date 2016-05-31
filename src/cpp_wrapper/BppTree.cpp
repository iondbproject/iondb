/******************************************************************************/
/**
@file		BppTree.cpp
@author		Dana Klamut
@brief
*/
/******************************************************************************/

#include "BppTree.h"

BppTree::BppTree(
        key_type_t type_key,
        int key_size,
        int value_size
) : CppDictionary(
        type_key, key_size,
        value_size
)
{
    bpptree_init(&handler);

    /** Where does this method get called? */
    dictionary_create(
        &handler,
        &dict,
        0,
        type_key,
        key_size,
        value_size,

        /** What is default dictionary size when it is not an input parameter? */
        0
    );
}