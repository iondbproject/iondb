/*
 * dicttypes.h
 *
 *  Created on: May 28, 2014
 *      Author: workstation
 */

#ifndef DICTTYPES_H_
#define DICTTYPES_H_

#include "./../system.h"
typedef char * key_t;
typedef char * value_t;
typedef char boolean_t;

typedef struct dictionary dictionary_t;
typedef struct dictionary_handler dictionary_handler_t;
typedef struct dictionary_cursor dict_cursor_t;

//FIXME
/**
 * A predicate is used with the find statement to produce a resultset with multiple values
*/
typedef struct predicate
{
	void * stuff;
}predicate_t;

/**
 * A dictionary_handler is responsible for dealing with the specific interface
 * for an underlying dictionary, but is decoupled from a specific collection
 */
struct dictionary_handler
{
	//what needs to go in here?
	err_t(* insert)(dictionary_t *, key_t *, value_t *);
	err_t(* create_dictionary)(int, int, int, dictionary_handler_t * , dictionary_t *);
	err_t(* get)(dictionary_t *, key_t *, value_t **);
	err_t(* update)(dictionary_t *, key_t *, value_t *);
	err_t(* find)(dictionary_t *, predicate_t *, dict_cursor_t *);
	//err_t(* find_g)(dictionary_t *, key_t *, key_t *, cursor_t **);	//min max
	//err_t(* next)(cursor_t *);
	err_t(* delete)(dictionary_t *, key_t *);
	err_t(* delete_dictionary)(dictionary_t *);
};
/**
 * A collection contains information regarding an instance of the storage element
 * and the associated handler.
 */
struct dictionary //dictionary TODO Rename
{
	void * instance;						// specific instance of a collection (but we don't know type)
	dictionary_handler_t * handler;			// handler for the specific type
};

/**
@brief
		Dictionary cursor type.
@details
		This is a super type. The state information
		must be stored within a subtype that makes
		sense to the particular dictionary implementation.
*/

typedef enum cursor_type
{
	equality,
	range,
	predicate
}cursor_type_t;

struct dictionary_cursor
{
	cursor_type_t type;
	status_t status;
	dictionary_t * dictionary;
};

typedef struct equality_cursor
{
	dict_cursor_t super;
	boolean_t(* equal)(dictionary_t *, key_t *);
}equality_cursor_t;

typedef struct range_cursor
{
	dict_cursor_t super;
	boolean_t(* range)(dictionary_t *, key_t *, key_t *);
}range_t;

typedef struct predicate_cursor
{
	dict_cursor_t super;
	boolean_t(* predicate)(dictionary_t *, void *);			// TODO FIXME the void * needs to be dealt with
}predicate_cursor_t;


#endif /* DICTTYPES_H_ */
