/******************************************************************************/
/**
@file		CppDictionary.cpp
@author		Dana Klamut
@brief      Interface describing how user interacts with general dictionaries
            using C++.
*/
/******************************************************************************/

#include "CppDictionary.h"
#include "../dictionary/dictionary_types.h"

CppDictionary::CppDictionary(
        key_type_t type_key,
        int key_size,
        int value_size,
        int dictionary_size
)
{
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

/** If dictionary size isn't used by the implementation, what does it default to for the dictionary_create method? */
CppDictionary::CppDictionary(
        key_type_t type_key,
        int key_size,
        int value_size
)
{
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

template <typename V>
err_t
insert(
        key_type_t key,
        V value
)
{
    ion_key_t ion_key       = (ion_key_t)&key;
    ion_value_t ion_value   = (ion_value_t)&value;

    err_t err               = dictionary_insert(&dict, ion_key, ion_value);

    return err;
}

template <typename V>
err_t
get(
        key_type_t key,
        V **value
)
{
    *value                  = malloc(sizeof(V));

    ion_key_t ion_key       = (ion_key_t)&key;
    ion_value_t ion_value   = (ion_value_t)&value;

    err_t err               = dictionary_get(&dict, ion_key, ion_value);

    return err;
}

err_t
deleteKey(
        key_type_t key
)
{
    ion_key_t ion_key   = (ion_key_t)&key;
    err_t err           = dictionary_delete(&dict, ion_key);

    return err;
}

template <typename V>
err_t
update(
        key_type_t key,
        V value
)
{
    ion_key_t ion_key       = (ion_key_t)&key;
    ion_value_t ion_value   = (ion_value_t)&value;
    err_t err               = dictionary_update(&dict, ion_key, ion_value);

    return err;
}

err_t
destroy()
{
    err_t err = dictionary_delete_dictionary(&dict);

    return err;
}


/** Is config info of type ion_dictionary_config_info_t passed directly from user? */
err_t
open(
        ion_dictionary_config_info_t config_info
)
{
    err_t err = dictionary_open(&handler, &dict, &config_info);

    return err;
}

err_t
close()
{
    err_t err = dictionary_close(&dict);

    return err;
}

/** Needs code review */
template <typename V>
err_t
range(
        V min_value,
        V max_value
)
{
    predicate_t predicate;
    /** How is cursor initialized? */
    dict_cursor_t **cursor;

    ion_key_t ion_min_value = (ion_key_t)&min_value;
    ion_key_t ion_max_value = (ion_key_t)&max_value;

    dictionary_build_predicate(&predicate, predicate_range, ion_min_value, ion_max_value);

    err_t err               = dictionary_find(&dict, &predicate, cursor);

    return err;
}

/** Needs code review */
err_t
equality(
        key_type_t key
)
{
    predicate_t predicate;
    /** How is cursor initialized? */
    dict_cursor_t **cursor;

    ion_key_t ion_key       = (ion_key_t)&key;

    dictionary_build_predicate(&predicate, predicate_equality, ion_key);

    err_t err           = dictionary_find(&dict, &predicate, cursor);

    return err;
}

/** Needs code review */
err_t
allRecords()
{
    predicate_t predicate;
    /** How is cursor initialized? */
    dict_cursor_t **cursor;

    dictionary_build_predicate(&predicate, predicate_all_records);

    err_t err = dictionary_find(&dict, &predicate, cursor);

    return err;
}

