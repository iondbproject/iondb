/******************************************************************************/
/**
@file		
@author		Scott Fazackerley, Graeme Douglas
@brief		Types used for dictionaries.
@detials	This file exists mostly to avoid circular dependencies.
*/
/******************************************************************************/

#ifndef DICTTYPES_H_
#define DICTTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "./../kv_system.h"

/**
@brief		A dictionary key.
*/
typedef char						*ion_key_t;

/**
@brief		A dictionary value.
*/
typedef char						*ion_value_t;

// TODO
/**
@brief		A boolean.
@todo		Move to a common types file.
*/
typedef char						boolean_t;

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

//FIXME
/**
@todo A predicate is used with the find statement to produce a resultset with multiple values
*/
typedef struct predicate
{
	void	*stuff;
} predicate_t;

/**
@brief		A dictionary_handler is responsible for dealing with the specific interface
			for an underlying dictionary, but is decoupled from a specific collection
*/
struct dictionary_handler
{
	err_t	(* insert)(dictionary_t *, ion_key_t *, ion_value_t *);
		/**< A pointer to the dictionaries insertion function. */
	err_t	(* create_dictionary)(int, int, int, dictionary_handler_t * , dictionary_t *);
		/**< A pointer to the dictionaries creation function. */
	err_t	(* get)(dictionary_t *, ion_key_t *, ion_value_t **);
		/**< A pointer to the dictionaries get function. */
	err_t	(* update)(dictionary_t *, ion_key_t *, ion_value_t *);
		/**< A pointer to the dictionaries update function. */
	err_t	(* find)(dictionary_t *, predicate_t *, dict_cursor_t *);
	//err_t	(* find_g)(dictionary_t *, key_t *, key_t *, cursor_t **);	//min max
	//err_t	(* next)(cursor_t *);
	err_t	(* delete)(dictionary_t *, ion_key_t *);
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
typedef enum cursor_type
{
	equality,	/**< Equality cursor. */
	range,		/**< Range cursor. */
	predicate	/**< Predicate cursor. */
} cursor_type_t;

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
	status_t				status;			/**< Status of last cursor call. */
	dictionary_t			*dictionary;	/**< Reference to the dictionary
											     the cursor belongs to. */
};

/**
@brief		Dictionary cursor for equality queries.
@details	Used when a dictionary supports multiple vvalues for a given key.
			
			This subtype should be extended when supported for a given
			dictionary.
*/
typedef struct equality_cursor
{
	dict_cursor_t 	super;
		/**< Cursor supertype this type inherits from. */
	boolean_t		(* equal)(dictionary_t *, ion_key_t *);
		/**< A pointer to an equality function. */
} equality_cursor_t;

/**
@brief		Dictionary cursor for range queries.
@details	This subtype should be extended when supported
			for a given dictionary.
*/
typedef struct range_cursor
{
	dict_cursor_t	super;
		/**< Cursor supertype this type inherits from. */
	boolean_t		(* range)(dictionary_t *, ion_key_t *, ion_key_t *);
		/**< A pointer to a range function. */
} range_t;

/**
@brief		Dictionary cursor for equality queries.
@details	Used when a user gives a function pointer to evaluate
			over each record in the dictionary.
			
			This subtype should be extended when supported for a given
			dictionary.
*/
typedef struct predicate_cursor
{
	dict_cursor_t	super;
		/**< Cursor supertype this type inherits from. */
	boolean_t		(* predicate)(dictionary_t *, void *);			// TODO FIXME the void * needs to be dealt with
		/**< A pointer to function that that filters records. */
} predicate_cursor_t;


#ifdef __cplusplus
}
#endif

#endif /* DICTTYPES_H_ */
