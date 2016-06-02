//
// Created by Dana Klamut on 2016-05-31.
//

//#include "../BppTree.h"
#include "../CppDictionary.h"
//#include "../LinearHash.h"
//#include "../FlatFile.h"
//#include "../OpenAddressFileHash.h"

#include "../../kv_system.h"
#include "../../dictionary/dictionary_types.h"
#include "../BppTree.h"

int main() {

    int                     key_size, value_size;
    key_type_t              key_type;
    dictionary_handler_t    bpp_tree_handler;
    dictionary_t            dictionary;
    status_t                status;

    /* These sizes are given in units of bytes. */
    key_type 	= key_type_numeric_signed;
    key_size 	= 4;
    value_size 	= 14;

    /* Here, we make the assumption that the value is going to be a null-terminated string. */
    int	            key 		= 1;
    unsigned char   value[value_size];
    sprintf((char *) value, "Hello World 0");


    /* This writes to the master table record, and initializes the dictionary. */
    printf("Create the dictionary using a BPP Tree\n");
    BppTree *bppdict = new BppTree(key_type, key_size, value_size);
        printf("Created the dictionary\n");


    /*
        Insertions take a key and a value that have been properly ionized. "Ionization" is the process of converting
         a key or value from its normal state, into a "byte array". The layout of the data is still exactly the same,
         but ionization erases type information to allow IonDB to remain type-agnostic.
    */
    printf("Inserting (%d|%s)...\n", key, value);
    status = bppdict->insert((key_type_t) key, value);
    if (status != err_ok) {
        printf("Inserting (%d|%s) failed\n", key, value);
        return 1;
    }

    /* Deletion removes all records stored with the given key (Since duplicate keys are allowed). */
    printf("Deleting (%d)...\n", key);
    status = bppdict->deleteKey((key_type_t) key);
    if (status != err_ok) {
        printf("Deleting (%d) failed\n", key);
        return 1;
    }

    /*
        Updates take a new value, and replaces all values stored at the given key, similar to how delete works.
        If the key doesn't currently exist, it will be inserted instead of being updated.
    */
    unsigned char replace_value[value_size];
    sprintf((char *) replace_value, "Hello new!");
    printf("Updating (%d with %s)...\n", key, replace_value);
    status = bppdict->update((key_type_t) key, replace_value);
    if (status != err_ok) {
        printf("Updating (%d with %s) failed\n", key, replace_value);
        return 1;
    }

    /* 4 Records are inserted to demonstrate the cursor queries. */
    int i;
    for (i = 2; i < 6; i++) {
        sprintf((char *) value, "Hello World %d", i);
        printf("Inserting (%d|%s)...\n", key, value);
        status = bppdict->insert((key_type_t) key, value);

        if (status != err_ok) {
            printf("Inserting (%d|%s) failed\n", key, value);
            return 1;
        }
    }

    /* A get operation requires the user to allocate space to store the returned value. Only retrieves one record. */
    printf("Querying (%d)...\n", key);
    unsigned char new_value[value_size];
    status = bppdict->get((key_type_t) key, new_value);
    if (status == err_ok) {
        printf("Got the value back of '%s' stored in %d.\n", value, key);
    }
    else {
        printf("Failed to query (%d)\n", key);
        return 1;
    }

    /*
        First step to a cursor query is to initialize the predicate. How this is done depends on the type of query.
        For the equality query, we're asking for all records with an exact match to the given key to be returned.
    */
    printf("Iterating through all records with key = %d...\n", key);
    predicate_t predicate;
    if (bppdict->equality((key_type_t) key) != err_ok) {
        printf("Failed to build predicate\n");
        return 1;
    }

    printf("Closing dictionary...\n");
    if(bppdict->close() != err_ok) {
        printf("Failed to close dictionary\n");
        return 1;
    }

    /* Clean-up the dictionary. Must be done through the master table to erase the table record we kept. */
    printf("Deleting dictionary...\n");
    if (bppdict->destroy() != err_ok) {
        printf("Failed to delete the dictionary\n");
        return 1;
    }


    printf("Done\n");
    return 0;

}