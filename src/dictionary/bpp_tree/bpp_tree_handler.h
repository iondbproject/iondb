/******************************************************************************/
/**
@file
@author		Graeme Douglas
@brief		The handler for a B+ tree.
*/
/******************************************************************************/

#if !defined(BPP_TREE_HANDLER_H_)
#define BPP_TREE_HANDLER_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary_types.h"
#include "./../dictionary.h"
#include "../../key_value/kv_system.h"
#include "../../file/linked_file_bag.h"
#include "bpp_tree.h"

typedef struct bplusplustree {
	ion_dictionary_parent_t super;
	ion_bpp_handle_t		tree;
	ion_lfb_t				values;
} ion_bpptree_t;

typedef struct {
	ion_dict_cursor_t	super;		/**< Supertype of cursor		*/
	ion_key_t			cur_key;/**< Current key we're visiting */
	ion_file_offset_t	offset;		/**< offset in LFB; holds value */
} ion_bpp_cursor_t;

/**
@brief		Registers a specific handler for a  dictionary instance.

@details	Registers functions for handlers.  This only needs to be called
			once for each type of dictionary that is present.

@param	  handler
				The handler for the dictionary instance that is to be
				initialized.
*/
void
bpptree_init(
	ion_dictionary_handler_t *handler
);

/**
@brief		Inserts a @p key and @p value into the dictionary.

@param	  dictionary
				The dictionary instance to insert the value into.
@param	  key
				The key to use.
@param	  value
				The value to use.
@return		The status on the insertion of the record.
*/
ion_status_t
bpptree_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief	  Queries a dictionary instance for the given @p key and returns
			the associated @p value.

@details	Queries a dictionary instance for the given @p key and returns
			the associated @p value.  If the @p write_concern is set to
			wc_insert_unique then if the @p key exists already, an error will
			be generated as duplicate keys are prevented.  If the
			@p write_concern is set to wc_update, the updates are allowed.
			In this case, if the @p key exists in the hashmap, the @p value
			will be updated.  If the @p key does not exist, then a new item
			will be inserted to hashmap.

@param	  dictionary
				The instance of the dictionary to query.
@param	  key
				The key to search for.
@param	  value
				A pointer that is used to return the value associated with
				the provided key.  The function will malloc memory for the
				value and it is up to the consumer the free the associated
				memory.
@return		The status of the query.
*/
ion_status_t
bpptree_query(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief		Creates an instance of a dictionary.

@details	Creates as instance of a dictionary given a @p key_size and
			@p value_size, in bytes. The @p dictionary_size parameter is
			not used for this implementation, as there is no size bound.
@param		id
				ID of a dictionary that's given to us.
@param		key_type
				The key category given to us.
@param		key_size
				The size of the key in bytes.
@param		value_size
				The size of the value in bytes.
@param		dictionary_size
				The size of the hashmap in discrete units
@param		compare
				Function pointer for the comparison function for the dictionary.
@param		handler
				 THe handler for the specific dictionary being created.
@param		dictionary
				 The pointer declared by the caller that will reference
				 the instance of the dictionary created.
@return		The status of the creation of the dictionary.
*/
ion_err_t
bpptree_create_dictionary(
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	int							key_size,
	int							value_size,
	unsigned int				dictionary_size,
	ion_dictionary_compare_t	compare,
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary
);

/**
@brief		Deletes the @p key and assoicated value from the dictionary
			instance.

@param	  dictionary
				The instance of the dictionary to delete from.
@param	  key
				The key that is to be deleted.
@return		The status of the deletion
*/
ion_status_t
bpptree_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
);

/**
@brief	  Deletes an instance of the dictionary and associated data.

@param	  dictionary
				The instance of the dictionary to delete.
@return		The status of the dictionary deletion.
*/
ion_err_t
bpptree_delete_dictionary(
	ion_dictionary_t *dictionary
);

/**
@brief		Updates the value for a given key.

@details	Updates the value for a given @p key.  If the key does not currently
			exist in the hashmap, it will be created and the value sorted.

@param	  dictionary
				The instance of the dictionary to be updated.
@param	  key
				The key that is to be updated.
@param	  value
				The value that is to be updated.
@return		The status of the update.
*/
ion_status_t
bpptree_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief	  Finds multiple instances of a keys that satisfy the provided
			 predicate in the dictionary.

@details	Generates a cursor that allows the traversal of items where
			the items key satisfies the @p predicate (if the underlying
			implementation allows it).

@param	  dictionary
				The instance of the dictionary to search.
@param	  predicate
				The predicate to be used as the condition for matching.
@param	  cursor
				The pointer to a cursor which is caller declared but callee
				is responsible for populating.
@return		The status of the operation.
*/
ion_err_t
bpptree_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
);

/**
@brief		Next function to query and retrieve the next
			<K,V> that stratifies the predicate of the cursor.

@param		cursor
				The cursor to iterate over the results.
@param		record
				The structure used to hold the returned key value
				pair. This must be properly initialized and allocated
				by the user.
@return		The status of the cursor.
*/
ion_cursor_status_t
bpptree_next(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
);

/**
@brief		Destroys the cursor.

@details	Destroys the cursor when the user is finished with it.  The
			destroy function will free up internally allocated memory as well
			as freeing up any reference to the cursor itself.  Cursor pointers
			will be set to NULL as per ION_DB specification for de-allocated
			pointers.

@param	  cursor
				** pointer to cursor.
*/
void
bpptree_destroy_cursor(
	ion_dict_cursor_t **cursor
);

/**
@brief		Tests the supplied @p key against the predicate registered in the
			cursor.

@param	  cursor
				The cursor and predicate being used to test @p key against.
@param	  key
				The key to test.
@return		The result is the key passes or fails the predicate test.
*/
ion_boolean_t
bpptree_test_predicate(
	ion_dict_cursor_t	*cursor,
	ion_key_t			key
);

/* TODO Write me doc! */
ion_err_t
bpptree_open_dictionary(
	ion_dictionary_handler_t		*handler,
	ion_dictionary_t				*dictionary,
	ion_dictionary_config_info_t	*config,
	ion_dictionary_compare_t		compare
);

/* TODO Write me doc! */
ion_err_t
bpptree_close_dictionary(
	ion_dictionary_t *dictionary
);

#if defined(__cplusplus)
}
#endif

#endif /* BPP_TREE_HANDLER_H_ */
