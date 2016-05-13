/******************************************************************************/
/**
@file		bpptreehandler.h
@author		Graeme Douglas
@brief		The handler for a B+ tree.
*/
/******************************************************************************/

#ifndef BPPTREEHANDLER_H_
#define BPPTREEHANDLER_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include "../dictionary_types.h"
#include "./../dictionary.h"
#include "./../../kv_system.h"
#include "../../file/linked_file_bag.h"
#include "bpp_tree.h"

typedef struct bplusplustree
{
	dictionary_parent_t	super;
	bHandleType		tree;
	lfb_t			values;
} bpptree_t;

typedef struct {
    dict_cursor_t   super;  	/**< Supertype of cursor 		*/
    ion_key_t 		cur_key; 	/**< Current key we're visiting */
    file_offset_t   offset; 	/**< offset in LFB; holds value */
} bCursorType;

/**
@brief		Registers a specific handler for a  dictionary instance.

@details	Registers functions for handlers.  This only needs to be called
			once for each type of dictionary that is present.

@param 		handler
				The handler for the dictionary instance that is to be
				initialized.
 */
void
bpptree_init(
	dictionary_handler_t 	*handler
);

/**
@brief		Inserts a @p key and @p value into the dictionary.

@param 		dictionary
				The dictionary instance to insert the value into.
@param 		key
				The key to use.
@param 		value
				The value to use.
@return		The status on the insertion of the record.
 */
err_t
bpptree_insert(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t 	value
);

/**
@brief 		Queries a dictionary instance for the given @p key and returns
			the associated @p value.

@details	Queries a dictionary instance for the given @p key and returns
			the associated @p value.  If the @p write_concern is set to
			wc_insert_unique then if the @key exists already, an error will
			be generated as duplicate keys are prevented.  If the
			@p write_concern is set to wc_update, the updates are allowed.
			In this case, if the @p key exists in the hashmap, the @p value
			will be updated.  If the @p key does not exist, then a new item
			will be inserted to hashmap.

@param 		dictionary
				The instance of the dictionary to query.
@param 		key
				The key to search for.
@param 		value
				A pointer that is used to return the value associated with
				the provided key.  The function will malloc memory for the
				value and it is up to the consumer the free the associated
				memory.
@return		The status of the query.
 */
err_t
bpptree_query(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t		value
);

/**
@brief		Creates an instance of a dictionary.

@details	Creates as instance of a dictionary given a @p key_size and
			@p value_size, in bytes as well as the @p dictionary size
 	 	 	which is the number of buckets available in the hashmap.

@param 		key_size
				The size of the key in bytes.
@param 		value_size
				The size of the value in bytes.
@param 		dictionary_size
				The size of the hashmap in discrete units
@param		compare
				Function pointer for the comparison function for the collection.
@param 		handler
 	 	 	 	 THe handler for the specific dictionary being created.
@param 		dictionary
 	 	 	 	 The pointer declared by the caller that will reference
 	 	 	 	 the instance of the dictionary created.
@return		The status of the creation of the dictionary.
 */
err_t
bpptree_create_dictionary(
		ion_dictionary_id_t 		id,
		key_type_t					key_type,
		int 						key_size,
		int 						value_size,
		int 						dictionary_size,
		ion_dictionary_compare_t 	compare,
		dictionary_handler_t 		*handler,
		dictionary_t 				*dictionary
);

/**
@brief		Deletes the @p key and assoicated value from the dictionary
			instance.

@param 		dictionary
				The instance of the dictionary to delete from.
@param 		key
				The key that is to be deleted.
@return		The status of the deletion
 */
err_t
bpptree_delete(
		dictionary_t 	*dictionary,
		ion_key_t 		key
);

/**
@brief 		Deletes an instance of the dictionary and associated data.

@param 		dictionary
				The instance of the dictionary to delete.
@return		The status of the dictionary deletion.
 */
err_t
bpptree_delete_dictionary(
		dictionary_t 	*dictionary
);

/**
@brief		Updates the value for a given key.

@details	Updates the value for a given @pkey.  If the key does not currently
			exist in the hashmap, it will be created and the value sorted.

@param 		dictionary
				The instance of the dictionary to be updated.
@param 		key
				The key that is to be updated.
@param 		value
				The value that is to be updated.
@return		The status of the update.
 */
err_t
bpptree_update(
		dictionary_t 	*dictionary,
		ion_key_t 		key,
		ion_value_t 	value
);

/**
@brief 		Finds multiple instances of a keys that satisfy the provided
 	 	 	 predicate in the dictionary.

@details 	Generates a cursor that allows the traversal of items where
			the items key satisfies the @p predicate (if the underlying
			implementation allows it).

@param 		dictionary
				The instance of the dictionary to search.
@param 		predicate
				The predicate to be used as the condition for matching.
@param 		cursor
				The pointer to a cursor which is caller declared but callee
				is responsible for populating.
@return		The status of the operation.
 */
err_t
bpptree_find(
		dictionary_t 	*dictionary,
		predicate_t 	*predicate,
		dict_cursor_t 	**cursor
);

/**
@brief		Next function to query and retrieve the next
			<K,V> that stratifies the predicate of the cursor.

@param 		cursor
				The cursor to iterate over the results.
@return		The status of the cursor.
 */
cursor_status_t
bpptree_next(
	dict_cursor_t 	*cursor,
	ion_record_t	*record
);

/**
@brief		Destroys the cursor.

@details	Destroys the cursor when the user is finished with it.  The
			destroy function will free up internally allocated memory as well
			as freeing up any reference to the cursor itself.  Cursor pointers
			will be set to NULL as per ION_DB specification for de-allocated
			pointers.

@param 		cursor
				** pointer to cursor.
 */
void
bpptree_destroy_cursor(
	dict_cursor_t	 **cursor
);

/**
@brief		Tests the supplied @pkey against the predicate registered in the
			cursor.

@param 		cursor
				The cursor and predicate being used to test @pkey against.
@param 		key
				The key to test.
@return		The result is the key passes or fails the predicate test.
 */
boolean_t
bpptree_test_predicate(
    dict_cursor_t* 	cursor,
    ion_key_t 			key
);

/* TODO Write me doc! */
err_t
bpptree_open_dictionary(
	dictionary_handler_t 			*handler,
	dictionary_t 					*dictionary,
	ion_dictionary_config_info_t 	*config,
	ion_dictionary_compare_t	 	compare
);

/* TODO Write me doc! */
err_t
bpptree_close_dictionary(
	dictionary_t 					*dictionary
);

#ifdef  __cplusplus
}
#endif

#endif /* OADICTIONARYHANDLER_H_ */
