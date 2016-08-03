/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Handler liaison between dictionary API and skiplist implementation
*/
/******************************************************************************/
#if !defined(SKIP_LIST_HANDLER_H_)
#define SKIP_LIST_HANDLER_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "skip_list_types.h"
#include "skip_list.h"

/**
@brief	  Registers a skiplist handler to a dictionary instance.

@details	Binds each unique skiplist function to the generic dictionary
			interface. Only needs to be called once when the skiplist is
			initialized.

@param	  handler
				An instance of a dictionary handler that is to be bound.
				It is assumed @p handler is initialized by the user.
 */
void
sldict_init(
	ion_dictionary_handler_t *handler
);

/**
@brief	  Inserts a @p key and @p value pair into the dictionary.

@param	  dictionary
				The dictionary instance to insert the value into.
@param	  key
				The key to use.
@param	  value
				The value to use.
@return	 Status of insertion.
 */
ion_status_t
sldict_insert(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief	  Queries a dictionary instance for a given @p key and returns the
			corresponding @p value.

@details	Queries a dictionary instance for a given @p key and returns the
			@p value within, copied into the pointer provided by the user.
			Assumption is that the pointer is passed unallocated, for this
			function to allocate. The responsibility is then on the user to
			free the given memory.

@param	  dictionary
				The instance of the dictionary to query
@param	  key
				The key to search for.
@param	  value
				A pointer used to hold the returned value from the query. The
				memory for value is assumed to be allocated and freed by the
				user.
@return	 Status of query.
 */
ion_status_t
sldict_query(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief	  Creates an instance of a dictionary.

@details	Creates an instance of a dictionary given a @p key_size and
			@p value_size, in bytes as well as the @p dictionary_size, which
			is the maximum number of levels in the skiplist. By nature of the
			structure, the maximum number of elements is bounded only by memory
			use.

@param		id
@param		key_type
@param	  key_size
				Size of the key in bytes.
@param	  value_size
				Size of the value in bytes.
@param		dictionary_size
@param		compare
@param	  handler
				Handler to be bound to the dictionary instance being created.
				Assumption is that the handler has been initialized prior.
@param	  dictionary
				Pointer in which the created dictionary instance is to be
				stored. Assumption is that it has been properly allocated by
				the user.
@return	 Status of creation.
 */
ion_err_t
sldict_create_dictionary(
	ion_dictionary_id_t			id,
	ion_key_type_t				key_type,
	int							key_size,
	int							value_size,
	int							dictionary_size,
	ion_dictionary_compare_t	compare,
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dictionary
);

/**
@brief	  Deletes the @p key and associated value from the given dictionary
			instance.

@param	  dictionary
				The instance of the dictionary to delete from.
@param	  key
				The key to be deleted.
@return	 Status of deletion.
 */
ion_status_t
sldict_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
);

/**
@brief	  Deletes an instance of a dictionary and its associated data.

@param	  dictionary
				The instance of the dictionary to be deleted.
@return	 Status of dictionary deletion.
 */
ion_err_t
sldict_delete_dictionary(
	ion_dictionary_t *dictionary
);

/**
@brief	  Updates the value stored at a given key.

@details	Updates the value for a given @p key. If the key doesn't exist,
			the key value pair will be added as if it was an insert.

@param	  dictionary
				The instance of the dictionary to be updated.
@param	  key
				The key that is to be updated.
@param	  value
				The new value to be used.
@return Status of update.
 */
ion_status_t
sldict_update(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief	  Finds multiple keys based on the provided predicate.

@details	Finds multiple keys based on the provided predicate. Gives a cursor
			that allows traversal of all key/value pairs that satisfy the
			@p predicate. Not all implementations support a find.

@param	  dictionary
				The instance of a dictionary to search within.
@param	  predicate
				The predicate used to match.
@param	  cursor
				The pointer to a cursor declared by the caller, but initialized
				and populated within the function.
@return	 Status of find.
 */
ion_err_t
sldict_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
);

/**
@brief	  Next function queries and retrieves the next key/value pair that
			satisfies the predicate of the cursor.

@param	  cursor
				The cursor used to iterate over results.
@param	  record
				A record pointer that is allocated by the caller in which the
				cursor will fill with the next key/value result. The assumption
				is that the caller will also free this memory.
@return	 Status of cursor.
 */
ion_cursor_status_t
sldict_next(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
);

/**
@brief	  Destroys the cursor.

@details	Destroys the cursor when the user is finished with it. All memory
			internally used by the cursor is freed as well. Cursor pointers
			will be set to NULL as per IonDB specification.

@param	  cursor
				Pointer to a pointer of a cursor.
 */
void
sldict_destroy_cursor(
	ion_dict_cursor_t **cursor
);

/**
@brief	  Tests a given @p key against the predicate of a cursor.

@param	  cursor
				Cursor in which to test against the @p key.
@param	  key
				Key to test.
@return	 Result of predicate comparison.
 */
ion_boolean_t
sldict_test_predicate(
	ion_dict_cursor_t	*cursor,
	ion_key_t			key
);

/**
@brief			Opens a specific skiplist instance of a dictionary.

@param			handler
					A pointer to the handler for the specific dictionary being opened.
@param			dictionary
					The pointer declared by the caller that will reference
					the instance of the dictionary opened.
@param			config
					The configuration info of the specific dictionary to be opened.
@param			compare
					Function pointer for the comparison function for the dictionary.

@return			The status of opening the dictionary.
 */
ion_err_t
sldict_open_dictionary(
	ion_dictionary_handler_t		*handler,
	ion_dictionary_t				*dictionary,
	ion_dictionary_config_info_t	*config,
	ion_dictionary_compare_t		compare
);

/**
@brief			Closes a skiplist instance of a dictionary.

@param			dictionary
					A pointer to the specific dictionary instance to be closed.

@return			The status of closing the dictionary.
 */
ion_err_t
sldict_close_dictionary(
	ion_dictionary_t *dictionary
);

#if defined(__cplusplus)
}
#endif

#endif /* SKIP_LIST_HANDLER_H_ */
