/******************************************************************************/
/**
@file
@author		Scott Fazackerley, Graeme Douglas
@brief		Types used for dictionaries.
@details	This file exists mostly to avoid circular dependencies.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see @ref AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
*/
/******************************************************************************/

#if !defined(DICTIONARY_TYPES_H_)
#define DICTIONARY_TYPES_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "./../kv_system.h"

/**
@brief	  A type used to identify dictionaries, specifically in the master
			table.
*/
typedef unsigned int ion_dictionary_id_t;

/**
@brief		A type describing how a dictionary is used.
@details	This type allows users of the library to find certain
			dictionaries in the key-value store that might be special.
			See @ref ion_find_by_use_master_table.
*/
typedef ion_byte_t ion_dict_use_t;

/**
@brief		Struct containing details for opening a dictionary previously
			created.
*/
typedef struct {
	ion_dictionary_id_t		id;					/**< The identifier used to
													 identify the dictionary. */
	ion_dict_use_t			use_type;			/**< How the dictionary will be
													 used. Ignore if N/A. */
	key_type_t				type;				/**< The type of key to store.
												*/
	ion_key_size_t			key_size;			/**< The size of the key. */
	ion_value_size_t		value_size;			/**< The size of the value. */
	ion_dictionary_size_t	dictionary_size;	/**< The dictionary size
													 parameter. Dependent on
													 the dictionary
													 implementation used. */
} ion_dictionary_config_info_t;

/**
@brief		A dictionary instance variable.
@details	Does not describe the function pointers of a dictionary
			implementation.
@see		dictionary
*/
typedef struct dictionary dictionary_t;

/**
@brief		Struct defining all dictionary handling methods.
@see		dictionary_handler
*/
typedef struct dictionary_handler dictionary_handler_t;

/**
@brief	Function pointer type for dictionary comparison methods.
*/
typedef char (*ion_dictionary_compare_t)(
	ion_key_t,
	ion_key_t,
	ion_key_size_t
);

/**
@brief		The dictionary cursor type.
@see		dictionary_cursor
*/
typedef struct dictionary_cursor dict_cursor_t;

/**
@brief		The dictionary predicate type.
@see		predicate
*/
typedef struct predicate predicate_t;

/**
@brief		The dictionary predicate statement type.
@see		predicate_statement
*/
typedef union predicate_statement predicate_statement_t;

/**
@brief		The dictionary parent type.
@see		dictionary_parent
*/
typedef struct dictionary_parent dictionary_parent_t;

/**
@brief		A comparison result type that describes the result of a comparison.
*/
typedef enum comparison {
	A_lt_B	= -1,	/**< The result for the comparison operation is A <= B. */
	A_equ_B = 0,/**< The result for the comparison operation is A == B. */
	A_gt_B	= 1		/**< The result for the comparison operation is A >= B. */
} comparsion_t;

/**
@brief		A status type describing the current state of an initialized cursor.
*/
enum cursor_status {
	cs_invalid_index = -1,	/**< A cursor status stating that
											 the cursor has an invalid index. */
	cs_invalid_cursor,	/**< A cursor status stating that the
											 cursor is generally invalid. */
	cs_end_of_results,	/**< A cursor status stating that the
											 the cursor has reached the end of
											 the results. */
	cs_cursor_initialized,	/**< A cursor status stating that the
											 cursor has been intialized but has
											 data thats hasn't yet been
											 accessed.
											 Cursor is valid but data has need
											 been accessed. */
	cs_cursor_uninitialized,/**< A cursor status stating that
											 the cursor has not yet been
											 attached to a predicate statement
											 and associated data. */
	cs_cursor_active,	/**< A cursor status stating that
											 the cursor is active and is
											 traversing data. */
	cs_possible_data_inconsistency,	/**< A cursor status stating that the
											 data in the underlying dictionary
											 has been changed, making the cursor
											 invalid. */
	cs_valid_data	/**< The data in the cursor is valid. @todo we should delete this. */
};

/**
@brief		A type for the status of a cursor.
@details	This allows us to control the size of the status type,
			rather than depending on the enum.
*/
typedef char cursor_status_t;

/**
@brief		A dictionary_handler is responsible for dealing with the specific
			interface for an underlying dictionary, but is decoupled from a
			specific collection.
*/
struct dictionary_handler {
	ion_status_t (*insert)(
		dictionary_t *,
		ion_key_t,
		ion_value_t
	);
	/**< A pointer to the dictionaries insertion function. */
	err_t (*create_dictionary)(
		ion_dictionary_id_t,
		key_type_t,
		int,
		int,
		int,
		ion_dictionary_compare_t,
		dictionary_handler_t *,
		dictionary_t *
	);
	/**< A pointer to the dictionaries creation function. */
	ion_status_t (*get)(
		dictionary_t *,
		ion_key_t,
		ion_value_t
	);
	/**< A pointer to the dictionaries get function. */
	ion_status_t (*update)(
		dictionary_t *,
		ion_key_t,
		ion_value_t
	);
	/**< A pointer to the dictionaries update function. */
	err_t (*find)(
		dictionary_t *,
		predicate_t *,
		dict_cursor_t **
	);
	/**< A pointer to the dictionaries find function */
	ion_status_t (*remove)(
		dictionary_t *,
		ion_key_t
	);
	/**< A pointer to the dictionaries key-value deletion function. */
	err_t (*delete_dictionary)(
		dictionary_t *
	);
	/**< A pointer to the dictionaries dictionary removal function. */
	err_t (*open_dictionary)(
		dictionary_handler_t *,
		dictionary_t *,
		ion_dictionary_config_info_t *,
		ion_dictionary_compare_t
	);
	/**< A pointer to the dictionaries open function. */
	err_t (*close_dictionary)(
		dictionary_t *
	);
	/**< A pointer to the dictionaries close function */
};

/**
@brief		A dictionary contains information regarding an instance of the
			storage element and the associated handler.
*/
struct dictionary {
	dictionary_parent_t		*instance;	/**< Specific instance of a
											 collection (but we don't
											 know type). */
	dictionary_handler_t	*handler;	/**< Handler for the specific type. */
};

/**
@brief	  This is the super type for all dictionaries.
*/
struct dictionary_parent {
	key_type_t					key_type;	/**< The key type stored in the map. */
	record_info_t				record;	/**< The record structure for items. */
	ion_dictionary_compare_t	compare;/**< Comparison function for
											  instance of map. */
	ion_dictionary_id_t			id;		/**< ID of dictionary instance. */
};

/**
@brief		A type for storing predicate type data.
*/
typedef char predicate_type_t;

/**
@brief		The predicate type flag list.
@details	These type flags tell internal code what type of predicate is
			being described.
*/
enum predicate_type {
	predicate_equality,	/**< Predicate type for equality cursors. */
	predicate_range,/**< Predicate tyoe for range cursors. */
	predicate_all_records,	/**< Predicate type for cursors over all records. */
	predicate_predicate	/**< Predicate type for predicate cursors. */
};

/**
@brief		This is a predicate data object for equality queries.
@details	This is to be used by the user to setup a predicate for evaluation.
*/
typedef struct equality_statement {
	ion_key_t equality_value;
	/**< The value to match in the equality. */
} equality_statement_t;

/**
@brief		This is a predicate data object for range queries.
@details	This is to be used by the user to setup a predicate for evaluation.
*/
typedef struct range_statement {
	ion_key_t	lower_bound;
	/**< The lower value in the range */
	ion_key_t	upper_bound;
	/**< The upper value in the range */
} range_statement_t;

/**
@brief		Predicate type for cursors that iterate over all records in set.
@details	This is to be used by the user to setup a predicate for evaluation.
*/
typedef struct ion_all_records_statement {
	/**> For now, all_records has no required information. */
	char unused;
} ion_all_records_statement_t;

/**
@brief		Predicate type for predicate (conditional) queries.
@details	This is to be used by the user to setup a predicate for evaluation.
*/
typedef struct other_predicate_statement {
	/**> For now, other_predicate has no required information. */
	char unused;
} other_predicate_statement_t;

/**
@brief		This is used to pass predicate into a cursor-based query over
			a dictionary.
*/
union predicate_statement {
	/**> An equality predicate statement. */
	equality_statement_t		equality;
	/**> A range predicate statement. */
	range_statement_t			range;
	/**> A general predicate statement. */
	other_predicate_statement_t other_predicate;
	/**> An all records predicate statement. */
	ion_all_records_statement_t all_records;
};

/**
@brief		A supertype for cursor predicate objects.
@details	This is a super type. The state information
			must be stored within a subtype that makes
			sense to the particular dictionary implementation.

			There are different types of predicates for different types of
			dictionary operations.

			A predicate is used with the find statement to produce a
			collection with multiple values.
*/
struct predicate {
	/**> Predicate type data for this predicate. */
	predicate_type_t		type;
	/**> Predicate statement data. This is specific to the type of predicate. */
	predicate_statement_t	statement;

	/**> A function pointer used to later free memory associated with the
		 predicate. */
	void (*destroy)(
		predicate_t **
	);
};

/**
@brief		A supertype for dictionary cursor objects.
@details	This is a super type. The state information
			must be stored within a subtype that makes
			sense to the particular dictionary implementation.

			There are different types of cursors for different types of
			dictionary operations.
*/
struct dictionary_cursor {
	cursor_status_t status;	/**< Status of last cursor call. */
	dictionary_t	*dictionary;			/**< A pointer to the dictionary
												 object. */
	predicate_t		*predicate;				/**< The predicate for the cursor.
											*/

	cursor_status_t (*next)(
		dict_cursor_t *,
		ion_record_t *record
	);
	/**< A pointer to the next function,
		 which sets cursor_status_t). */
	void (*destroy)(
		dict_cursor_t **
	);
	/**< A pointer to the function used
		 to destroy the cursor (frees
		 internal memory). */
};

/**
@brief		The list of write concern options for supported dictionary
			implementations.
@details	The write concern level describes dictionary behaviour when multiple
			values are inserted with the same key. Not all implementations are
			required to support different write concern levels.
*/
enum write_concern {
	wc_update,	/**< Write concern which allows for values to be
								 overwritten if their associated key
								 already exists in the dictionary. */
	wc_insert_unique,	/**< Write concern which allows for unique inserts
								 only (no overwrite). */
	wc_duplicate,	/**< Write concern which allows for duplicate keys
								 to be inserted into dictionary. */
};

/**
@brief		A type for write concern information used by hash table based
			dictionaries which limit insert/update of values.
*/
typedef char write_concern_t;

#if defined(__cplusplus)
}
#endif

#endif /* DICTIONARY_TYPES_H_ */
