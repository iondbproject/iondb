/******************************************************************************/
/**
@file		dictionary_library.h
@author		Dana Klamut
@brief
*/
/******************************************************************************/

#if !defined(PROJECT_CPP_DICTIONARY_H)
#define PROJECT_CPP_DICTIONARY_H

class cpp_dictionary {

protected:
    enum dictionary_type { bpp_tree, flat_file, linear_hash, open_address_file_hash, open_address_hash, skip_list };
    dictionary_handler_t handler;
    dictionary_t *dict;

public:
    cpp_dictionary(dictionary_type dict_type, key_type_t key_type, int key_size, int value_size, int dictionary_size)
    {
        switch(dict_type)
        {
            case bpp_tree :
                bpptree_init(&handler);
                break;

            case flat_file :
                ffdict_init(&handler);
                break;

            case linear_hash :
                lhdict_init(&handler);
                break;

            case open_address_file_hash :
                oafdict_init(&handler);
                break;

            case open_address_hash :
                oadict_init(&handler);
                break;

            case skip_list :
                sldict_init(&handler);
                break;
        }


        dictionary_create(
                &handler,
                dict,
                0, /** Should ID always be defaulted to 0? */
                key_type,
                key_size,
                value_size,
                dictionary_size
        );

    }

    template <typename T, typename V> void
    insert(T key, V value)
    {
        ion_key_t ion_key = &key;
        ion_value_t ion_value = &value;
        err_t err = dictionary_insert(dict, ion_key, ion_value);

        /** Do we want error messages displayed otherwise? */
        if(err_ok == err) { return; }
    }

    template <typename T, typename V> void
    get(T key, V value)
    {
        ion_key_t ion_key = &key;
        ion_value_t ion_value = &value;
        err_t err = dictionary_get(dict, ion_key, ion_value);

        /** Do we want error messages displayed otherwise? */
        if(err_ok == err) { return; }
    }

    template <typename T> void
    delete_key(T key)
    {
        ion_key_t ion_key = &key;
        err_t err = dictionary_delete(dict, ion_key);

        /** Do we want error messages displayed otherwise? */
        if(err_ok == err) { return; }
    }

    template <typename T, typename V> void
    update(T key, V value)
    {
        ion_key_t ion_key = &key;
        ion_value_t ion_value = &value;
        err_t err = dictionary_update(dict, ion_key, ion_value);

        /** Do we want error messages displayed otherwise? */
        if(err_ok == err) { return; }
    }

    void
    delete_dictionary()
    {
        err_t err = dictionary_delete_dictionary(dict);

        /** Do we want error messages displayed otherwise? */
        if(err_ok == err) { return; }
    }

    /** Is key size passed directly from user or determined by the method? */
    template <typename T, typename V> char
    compare_unsigned_value(T key_one, T key_two, V key_size)
    {
        ion_key_t ion_key_one = &key_one;
        ion_key_t ion_key_two = &key_two;
        ion_key_size_t ion_key_size = key_size;
        char return_value = dictionary_compare_unsigned_value(ion_key_one, ion_key_two, ion_key_size);

        return return_value;
    }

    /** Is key size passed directly from user or determined by the method? */
    template <typename T, typename V> char
    compare_signed_value(T key_one, T key_two, V key_size)
    {
        ion_key_t ion_key_one = &key_one;
        ion_key_t ion_key_two = &key_two;
        ion_key_size_t ion_key_size = key_size;
        char return_value = dictionary_compare_signed_value(ion_key_one, ion_key_two, ion_key_size);

        return return_value;
    }

    /** Is key size passed directly from user or determined by the method? */
    template <typename T, typename V> char
    compare_char_array(T key_one, T key_two, V key_size)
    {
        ion_key_t ion_key_one = &key_one;
        ion_key_t ion_key_two = &key_two;
        ion_key_size_t ion_key_size = key_size;
        char return_value = dictionary_compare_char_array(ion_key_one, ion_key_two, ion_key_size);

        return return_value;
    }

    /** Is key size passed directly from user or determined by the method? */
    template <typename T, typename V> char
    compare_null_terminated_string(T key_one, T key_two, V key_size)
    {
        ion_key_t ion_key_one = &key_one;
        ion_key_t ion_key_two = &key_two;
        ion_key_size_t ion_key_size = key_size;
        char return_value = dictionary_compare_null_terminated_string(ion_key_one, ion_key_two, ion_key_size);

        return return_value;
    }

    /** Is config info of type ion_dictionary_config_info_t passed directly from user? */
    void
    open(ion_dictionary_config_info_t config_info)
    {
        err_t err = dictionary_open(&handler, dict, &config_info);

        /** Do we want error messages displayed otherwise? */
        if(err_ok == err) { return; }
    }

    void
    close()
    {
        err_t err = dictionary_close(dict);

        /** Do we want error messages displayed otherwise? */
        if(err_ok == err) { return; }
    }

    /** Is predicate info of types predicate_t and predicate_type_t passed directly from user? */
    /** dictionary_build_predicate method is not complete in source file */
    void
    build_predicate(predicate_t predicate, predicate_type_t type)
    {
        err_t err = dictionary_build_predicate(&predicate, type);

        /** Do we want error messages displayed otherwise? */
        if(err_ok == err) { return; }
    }

    void
    destroy_predicate_equality(predicate_t **predicate)
    {
        dictionary_destroy_predicate_equality(predicate);
    }

    void
    destroy_predicate_range(predicate_t **predicate)
    {
        dictionary_destroy_predicate_range(predicate);
    }

    void
    destroy_predicate_all_records(predicate_t **predicate)
    {
        dictionary_destroy_predicate_all_records(predicate);
    }

    /** Is predicate info of type predicate_t and cursor of type dict_cursor_t to be passed directly from user? */
    void
    find(predicate_t predicate, dict_cursor_t **cursor)
    {
        err_t err = dictionary_find(dict, &predicate, cursor);

        /** Do we want error messages displayed otherwise? */
        if(err_ok == err) { return; }
    }

    /** Are there additional methods from classes other than dictionary.c that need to be implemented? */

};


#endif //PROJECT_CPP_DICTIONARY_H
