/******************************************************************************/
/**
@file		CppDictionary.h
@author		Dana Klamut
@brief      Interface describing how user interacts with general dictionaries
            using C++.
*/
/******************************************************************************/

#if !defined(PROJECT_CPP_DICTIONARY_H)
#define PROJECT_CPP_DICTIONARY_H

#include "../dictionary/dictionary.h"
#include "../dictionary/dictionary_types.h"

class CppDictionary {

protected:
    enum dictionary_type {
        BPP_TREE,
        FLAT_FILE,
        LINEAR_HASH,
        OPEN_ADDRESS_FILE_HASH,
        OPEN_ADDRESS_HASH,
        SKIP_LIST
    };

    dictionary_handler_t handler;
    dictionary_t dict;

public:
/* BEFORE USE OF A FILE BASED DICTIONARY IMPLEMENTATION (BPP_TREE, FLAT_FILE),
 * SD.begin(...) MUST BE CALLED. */

/**
@brief		General dictionary constructor for dictionaries that require
            dictionary size.
@param		key_t
				The type of keys to be stored in the dictionary.
@param		key_size
				The size of keys to be stored in the dictionary.
@param      value_size
                The size of the values to be stored in the dictionary.
@param      dictionary_size
                The size desired for the dictionary.
*/
CppDictionary(
            key_type_t type_key,
            int key_size,
            int value_size,
            int dictionary_size
    );

/**
@brief		General dictionary constructor for dictionaries that do not
            require dictionary size.
@param		key_t
				The type of keys to be stored in the dictionary.
@param		key_size
				The size of keys to be stored in the dictionary.
@param      value_size
                The size of the values to be stored in the dictionary.
*/
CppDictionary(
            key_type_t type_key,
            int key_size,
            int value_size
    );

/**
@brief		Insert a value into a dictionary.

@param		key
				The key that identifies @p value.
@param		value
				The value to store under @p key.
@returns	An error message describing the result of the insertion.
*/
    template <typename V>
    err_t
    insert(
            key_type_t key,
            V value
    );

/**
@brief		Retrieve a value given a key.

@details    A pointer-to-pointer variable must be created to
            pass into the method as an output parameter before
            the method is called.

@param		key
				The key to retrieve the value for.
@param		value
				A pointer to the value byte array to copy data into.
@return		An error message describing the result of the retrieval.
*/
    template <typename V>
    err_t
    get(
            key_type_t key,
            V **value
    );

/**
@brief		Delete a value given a key.

@param		key
				The key to be deleted.
@return		An error message describing the result of the deletion.
*/
    err_t
    deleteKey(
            key_type_t key
    );

/**
@brief		Update all records with a given key.

@param		key
				The key to identify records for updating.
@param		value
				The value to update records with.
@return     An error message describing the result of the update.
*/
    template <typename V>
    err_t
    update(
            key_type_t key,
            V value
    );

/**
@brief 		Destroys dictionary.

@return		An error message describing the total destruction of the dictionary.
 */
    err_t
    destroy();


    /** Is config info of type ion_dictionary_config_info_t passed directly from user? */
/**
@brief      Opens a dictionary, given the desired config.

@param      config_info
                The configuration of the dictionary to be opened.
@return     An error message describing the result of of the open.
*/
    err_t
    open(
            ion_dictionary_config_info_t config_info
    );

/**
@brief      Closes a dictionary.
*/
    err_t
    close();

/**
@brief      Performs a range query on a dictionary.

@param      min_value
                The minimum value to be included in the query.
@param      max_value
                The maximum value to be included in the query.
@returns    An error message describing the result of the query.
*/
    template <typename V>
    err_t
    range(
            V min_value,
            V max_value
    );

/**
@brief      Performs an equality query on a dictionary.

@param      key
                The key used to determine equality.
@returns    An error message describing the result of the query.
*/
    err_t
    equality(
            key_type_t key
    );

/**
@brief      Selects all records present in the dictionary.

@returns    An error message describing the result of the query.
*/
    err_t
    allRecords();

};


#endif //PROJECT_CPP_DICTIONARY_H
