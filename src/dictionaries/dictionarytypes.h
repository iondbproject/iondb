/******************************************************************************/
/**
@file		dictionarytypes.h
@author		Graeme Douglas
@brief		Dictionary type definitions.
@details	This file exists to avoid circular dependencies.
*/
/******************************************************************************/

#ifndef DICTIONARYTYPES_H
#define DICTIONARYTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief		The dictionary handler supertype.
@details	Should be the first member of any dictionary implementation.
		This handler should be agnostic of both the underlying
		underlying dictionary implementation and the storage
		medium used.
*/
struct dictionary
{
	chunk_handler_t		*chunk;		/**< The storage handler
						     to use with the
						     dictionary. */
	// TODO: Either store function pointers in the struct or use if
	//       statements and a type.
};
typedef struct dictionary dictionary_t;

/**
@brief		Dictionary cursor type.
@details	This is a super type. The state information
		must be stored within a subtype that makes
		sense to the particular dictionary implementation.
*/
struct dictionary_cursor
{
	status_t	status;		/**< The status of the most recent
					     iterator operation. */
	dictionary_t	*dict;		/**< A pointer to the dictionary
					     handler this iterator belongs
					     to. */
	my_key_t	key		/**< Key we are iterating over. */
};
typedef struct dictionary_iterator dict_cursor_t;

/**
@brief		Cursor type for @ref lpmemhashtable_t.
*/
struct lpmemhashtable_cursor
{
	dict_cursor_t	super;		/**< Super type of this cursor. */
	hashvalue_t	first;		/**< First visited spot. */
	hashvalue_t	current;	/**< Currently visited spot. */
};

#ifdef __cplusplus
}
#endif

#endif
