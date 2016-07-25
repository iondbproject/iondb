/******************************************************************************/
/**
@file
@author	 Scott Ronald Fazackerley
@brief	  The handler for a hash table using linear probing.
*/
/******************************************************************************/

#if !defined(OPEN_ADDRESS_DICTIONARY_HANDLER_H_)
#define OPEN_ADDRESS_DICTIONARY_HANDLER_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary_types.h"
#include "./../dictionary.h"
#include "../../key_value/kv_system.h"
#include "open_address_hash.h"
#include "open_address_hash_dictionary.h"

/**
@brief Struct used to for instance of a given dictionary.
 */
typedef struct oa_dictionary {
	/* what needs to go in here? */
	char			*dictionary_name;	/**<The name of the dictionary*/
	ion_hashmap_t	*hashmap;			/**<The map that the operations
										will operate upon*/
} ion_oa_dictionary_t;

/**
@brief	  Dictionary cursor for equality queries.
@details	Used when a dictionary supports multiple vvalues for a given key.

			This subtype should be extended when supported for a given
			dictionary.
*/
typedef struct oadict_equality_cursor {
	ion_dict_cursor_t	super;					/**<Super type this cursor inherits from*/
	ion_oadict_cursor_t cursor_info;/**<Super type to dict implementation*/
	ion_key_t			value;

	ion_boolean_t (*equal)(
		ion_dictionary_t *,
		ion_key_t,
		ion_key_t
	);
	/**< A pointer to an equality function. */
} ion_oadict_equality_cursor_t;

/**
@brief	  Registers a specific handler for a  dictionary instance.

@details	Registers functions for handlers.  This only needs to be called
			once for each type of dictionary that is present.

@param	  handler
				The handler for the dictionary instance that is to be
				initialized.
 */
void
oadict_init(
	ion_dictionary_handler_t *handler
);

/**
@brief	  Inserts a @p key and @p value into the dictionary.

@param	  dictionary
				The dictionary instance to insert the value into.
@param	  key
				The key to use.
@param	  value
				The value to use.
@return	 The status on the insertion of the record.
 */
ion_status_t
oadict_insert(
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
@return	 The status of the query.
 */
ion_status_t
oadict_query(
	ion_dictionary_t	*dictionary,
	ion_key_t			key,
	ion_value_t			value
);

/**
@brief	  Creates an instance of a dictionary.

@details	Creates as instance of a dictionary given a @p key_size and
			@p value_size, in bytes as well as the @p dictionary size
			which is the number of buckets available in the hashmap.

@param		id
@param		key_type
@param	  key_size
				The size of the key in bytes.
@param	  value_size
				The size of the value in bytes.
@param	  dictionary_size
				The size of the hashmap in discrete units
@param	  compare
				Function pointer for the comparison function for the dictionary.
@param	  handler
				 THe handler for the specific dictionary being created.
@param	  dictionary
				 The pointer declared by the caller that will reference
				 the instance of the dictionary created.
@return	 The status of the creation of the dictionary.
 */
ion_err_t
oadict_create_dictionary(
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
@brief	  Deletes the @p key and assoicated value from the dictionary
			instance.

@param	  dictionary
				The instance of the dictionary to delete from.
@param	  key
				The key that is to be deleted.
@return	 The status of the deletion
 */
ion_status_t
oadict_delete(
	ion_dictionary_t	*dictionary,
	ion_key_t			key
);

/**
@brief	  Deletes an instance of the dictionary and associated data.

@param	  dictionary
				The instance of the dictionary to delete.
@return	 The status of the dictionary deletion.
 */
ion_err_t
oadict_delete_dictionary(
	ion_dictionary_t *dictionary
);

/**
@brief	  Updates the value for a given key.

@details	Updates the value for a given @p key.  If the key does not currently
			exist in the hashmap, it will be created and the value sorted.

@param	  dictionary
				The instance of the dictionary to be updated.
@param	  key
				The key that is to be updated.
@param	  value
				The value that is to be updated.
@return	 The status of the update.
 */
ion_status_t
oadict_update(
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
@return	 The status of the operation.
 */
ion_err_t
oadict_find(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate,
	ion_dict_cursor_t	**cursor
);

/**
@brief	  Compares two key and returns the difference

@details	Compares two key and returns the difference depending on the type
			of the key defined for the dictionary instance.  If the keys are of numeric
			type, the return value is the difference between the keys.  If the
			value is negative, @p first_key is smaller than @p second_key.  If
			return value is positive, then @p first_key is larger than
			@p second_key.  If the return value is 0 then @p first_key is
			equal to @p second_key.

			If the key type is @p key_type_char_array then
			@todo fix this commemt!
			The function memcmp compares the size bytes of memory beginning at
			a1 against the size bytes of memory beginning at a2. The value
			returned has the same sign as the difference between the first
			differing pair of bytes (interpreted as unsigned char objects,
			then promoted to int).

@param	  first_key
				The first key in the comparison.
@param	  second_key
				The second key in the comparison.
@return	 The difference between the keys.
 */
int
oadict_compare(
	ion_key_t	first_key,
	ion_key_t	second_key
);

/**
@brief	  Next function to query and retrieve the next
			<K,V> that stratifies the predicate of the cursor.

@param	  cursor
				The cursor to iterate over the results.
@return	 The status of the cursor.
 */
/*ion_err_t
oadict_next(
	ion_dict_cursor_t   *cursor,
	ion_value_t	 *value
);*/

/**
@brief	  Next function to query and retrieve the next
			<K,V> that stratifies the predicate of the cursor.

@param	  cursor
				The cursor to iterate over the results.
@param		record
@return	 The status of the cursor.
 */
ion_cursor_status_t
oadict_next(
	ion_dict_cursor_t	*cursor,
	ion_record_t		*record
);

/**
@brief	  Compares two keys and determines if they are equal assuming
			that they are equal is length (in size).

@param	  dict
					 The map the keys are associated with.
@param	  key1
					 The first key for comparison.
@param	  key2
					 The second key for comparison.
@return	 If the keys are equal.
 */
ion_boolean_t
/*TODO Fix name of function */
is_equal(
	ion_dictionary_t	*dict,
	ion_key_t			key1,
	ion_key_t			key2
);

/**
@brief	  Destroys the cursor.

@details	Destroys the cursor when the user is finished with it.  The
			destroy function will free up internally allocated memory as well
			as freeing up any reference to the cursor itself.  Cursor pointers
			will be set to NULL as per ION_DB specification for de-allocated
			pointers.

@param	  cursor
				** pointer to cursor.
 */
void
oadict_destroy_cursor(
	ion_dict_cursor_t **cursor
);

/**
@brief	  Tests the supplied @p key against the predicate registered in the
			cursor.

@param	  cursor
				The cursor and predicate being used to test @p key against.
@param	  key
				The key to test.
@return	 The result is the key passes or fails the predicate test.
 */
ion_boolean_t
oadict_test_predicate(
	ion_dict_cursor_t	*cursor,
	ion_key_t			key
);

/**

@brief		  Starts scanning map looking for conditions that match
				predicate and returns result.

@details		Scans that map looking for the next value that satisfies the predicate.
				The next valid index is returned through the cursor

@param		  cursor
					A pointer to the cursor that is operating on the map.

@return		 The status of the scan.
 */
ion_err_t
oadict_scan(
	ion_oadict_cursor_t *cursor	/* don't need to pass in the cursor */
);

#if defined(__cplusplus)
}
#endif

#endif /* OPEN_ADDRESS_DICTIONARY_HANDLER_H_ */
