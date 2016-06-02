/******************************************************************************/
/**
@file		CppDictionary.cpp
@author		Dana Klamut
@brief      Interface describing how user interacts with general dictionaries
            using C++.
*/
/******************************************************************************/

#include "CppDictionary.h"
#include "../dictionary/ion_master_table.h"

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
        0 /** What value should this default to? */
    );

}

err_t
CppDictionary::insert(
        key_type_t key,
        key_type_t value
)
{
    ion_key_t ion_key       = (ion_key_t)&key;
    ion_value_t ion_value   = (ion_value_t)&value;

    err_t err               = dictionary_insert(&dict, ion_key, ion_value);

    return err;
}

err_t
CppDictionary::get(
        key_type_t key,
        key_type_t **value
)
{
    *value                  = (key_type_t *) malloc(sizeof(key_type_t));

    ion_key_t ion_key       = (ion_key_t)&key;
    ion_value_t ion_value   = (ion_value_t)&value;

    err_t err               = dictionary_get(&dict, ion_key, ion_value);

    return err;
}

err_t
CppDictionary::deleteKey(
        key_type_t key
)
{
    ion_key_t ion_key   = (ion_key_t)&key;
    err_t err           = dictionary_delete(&dict, ion_key);

    return err;
}

err_t
CppDictionary::update(
        key_type_t key,
        key_type_t value
)
{
    ion_key_t ion_key       = (ion_key_t)&key;
    ion_value_t ion_value   = (ion_value_t)&value;
    err_t err               = dictionary_update(&dict, ion_key, ion_value);

    return err;
}

err_t
CppDictionary::destroy()
{
    err_t err = dictionary_delete_dictionary(&dict);

    return err;
}


/** Is config info of type ion_dictionary_config_info_t passed directly from user? */
err_t
CppDictionary::open(
        ion_dictionary_config_info_t config_info
)
{
    err_t err = dictionary_open(&handler, &dict, &config_info);

    return err;
}

err_t
CppDictionary::close()
{
    err_t err = dictionary_close(&dict);

    return err;
}

/** Needs code review */
err_t
CppDictionary::range(
        key_type_t min_value,
        key_type_t max_value
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
CppDictionary::equality(
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
CppDictionary::allRecords()
{
    predicate_t predicate;
    /** How is cursor initialized? */
    dict_cursor_t **cursor;

    dictionary_build_predicate(&predicate, predicate_all_records);

    err_t err = dictionary_find(&dict, &predicate, cursor);

    return err;
}

err_t
masterTableLookup(
        unsigned int id,
        ion_dictionary_config_info_t *config
)
{
    err_t err           = ion_lookup_in_master_table(id, config);

    return err;
}

err_t
CppDictionary::masterTableOpenDictionary(
    unsigned int id
)
{
    err_t err = ion_open_dictionary(&handler, &dict, id);

    return err;
}

err_t
CppDictionary::masterTableCloseDictionary()
{
    err_t err = ion_close_dictionary(&dict);

    return err;
}

