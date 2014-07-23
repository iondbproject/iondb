/******************************************************************************/
/**
@file		
@author		Scott Fazackerley, Graeme Douglas
@brief		Types used for dictionaries.
@details	This file exists mostly to avoid circular dependencies.
*/
/******************************************************************************/

#ifndef DICTTYPES_H_
#define DICTTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "./../kv_system.h"


/**
@brief		A dictionary instance variable.
@details	Does not describe the function pointers of a dictionary
			implementation.
@see		@ref struct dictionary
*/
typedef struct dictionary			dictionary_t;

/**
@brief		Struct defining all dictionary handling methods.
@see		@ref struct dictionary_handler
*/
typedef struct dictionary_handler	dictionary_handler_t;

/**
@brief		The dictionary cursor type.
@see		@ref struct dictionary_cursor
*/
typedef struct dictionary_cursor	dict_cursor_t;

typedef struct predicate 			predicate_t;

typedef union predicate_statement	predicate_statement_t;

enum cursor_status
{
	cs_invalid_index = -1,					/**<Invalid index within cursor*/
	cs_invalid_cursor,						/**<Cursor is not valid */
	cs_end_of_results,						/**<cursor has reached end */
	cs_cursor_initialized,					/**<cursor is valid but data has need been accessed */
	cs_cursor_uninitialized,				/**<cursor is not atttaced to query */
	cs_cursor_active,						/**<cursor is active in data traversal */
	cs_possible_data_inconsistency,			/**<The collection has changed
												during the life of the
												cursor*/
	cs_valid_data
};

typedef char cursor_status_t;

/**
@brief		A dictionary_handler is responsible for dealing with the specific interface
			for an underlying dictionary, but is decoupled from a specific collection
*/
struct dictionary_handler
{
	err_t	(* insert)(dictionary_t *, ion_key_t, ion_value_t);
		/**< A pointer to the dictionaries insertion function. */
	err_t	(* create_dictionary)(key_type_t, int, int, int, char (* compare)(ion_key_t, ion_key_t, ion_key_size_t), dictionary_handler_t * , dictionary_t *);
		/**< A pointer to the dictionaries creation function. */
	err_t	(* get)(dictionary_t *, ion_key_t, ion_value_t *);
		/**< A pointer to the dictionaries get function. */
	err_t	(* update)(dictionary_t *, ion_key_t, ion_value_t);
		/**< A pointer to the dictionaries update function. */
	err_t	(* find)(dictionary_t *, predicate_t *, dict_cursor_t **);
	//err_t	(* find_g)(dictionary_t *, key_t *, key_t *, cursor_t **);	//min max
	//err_t	(* next)(cursor_t *);
	err_t	(* delete)(dictionary_t *, ion_key_t);
		/**< A pointer to the dictionaries key-value deletion function. */
	err_t	(* delete_dictionary)(dictionary_t *);
		/**< A pointer to the dictionaries dictionary removal function. */
};

/**
@brief		A dictionary contains information regarding an instance of the
			storage element and the associated handler.
*/
struct dictionary
{
	void					*instance;		/**< Specific instance of a
											     collection (but we don't
											     know type) */
	dictionary_handler_t 	*handler;		/**< Handler for the specific type.
											*/
};

/**
@brief		Dictionary cursor type designator.
*/
enum cursor_type
{
	cursor_equality,	/**< Equality cursor. */
	cursor_range,		/**< Range cursor. */
	cursor_predicate	/**< Predicate cursor. */
};

typedef char 		cursor_type_t;

/**
@brief		Predicate type designator.
*/
typedef char 						predicate_type_t;

enum predicate_type
{
	predicate_equality,		/**< Equality cursor. */
	predicate_range,		/**< Range cursor. */
	predicate_predicate		/**< Predicate cursor. */
};


/**
@brief		predicate for equality queries.
@details	Used by the user to setup a predicate for evaluation.
*/
typedef struct equality_statement
{
	ion_key_t		equality_value;
					/**< The value to match in the equality */
} equality_statement_t;

/**
@brief		predicate for range queries.
@details	Used by the user to setup a predicate for evaluation.
*/
typedef struct range_statement
{
	ion_key_t		leq_value;
					/**< The lower value in the range */
	ion_key_t		geq_value;
					/**< The upper value in the range */
} range_statement_t;


/**
@brief		predicate for predicate queries.
@details	Used by the user to setup a predicate for evaluation.
*/
typedef struct other_predicate_statement
{
					/** @TODO this needs to be resolved */
} other_predicate_statement_t;

/**
@brief used to pass predicate into query
 */
union predicate_statement
{
	equality_statement_t 			equality;
	range_statement_t				range;
	other_predicate_statement_t		other_predicate;
};

//FIXME
/**
@brief		Predicate supertype.
@details	This is a super type. The state information
			must be stored within a subtype that makes
			sense to the particular dictionary implementation.

			There are different types of predicates for different types of
			dictionary operations.

			@todo A predicate is used with the find statement to produce a
			collection with multiple values
*/
struct predicate
{
	predicate_type_t		type;
	predicate_statement_t 	statement;
};

/**
@brief		Dictionary cursor supertype.
@details	This is a super type. The state information
			must be stored within a subtype that makes
			sense to the particular dictionary implementation.

			There are different types of cursors for different types of
			dictionary operations.
*/
struct dictionary_cursor
{
	cursor_type_t			type;			/**< Cursor type designator. */
	cursor_status_t			status;			/**< Status of last cursor call. */
	dictionary_t			*dictionary;	/**< Reference to the dictionary */
	predicate_t				predicate;		/**< The predicate for the cursor */
	cursor_status_t			(* next)(dict_cursor_t *, ion_value_t value);
											/**< Next function binding *cursor_status_t)*/
	void					(* destroy)(dict_cursor_t **);
											/**< Destroy the cursor (frees internal memory) */
};

#ifdef __cplusplus
}
#endif

#endif /* DICTTYPES_H_ */
