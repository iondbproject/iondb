/******************************************************************************/
/**
@file 		sahandler.h
@author		Raffi Kudlac
@brief		The .h file for the sahandler.c file
*/
/******************************************************************************/
#ifndef SAHANDLER_H_
#define SAHANDLER_H_


#include "staticarray.h"
#include "./../../kv_system.h"
#include "./../dicttypes.h"
#include "./../dictionary.h"



/**
@brief		Used to initalize the handler

@param		handler
				a pointer to the handler that will be initalized

@return		The status of the initalization.
*/
status_t
sadict_init(
	dictionary_handler_t 	*handler
);


/**
@brief		Used to query the dictionary

@param		dictionary
				pointer to the dictionary that holds everything

@param		key
				a pointer to the key

@param		value
				a double pointer that will be set to the value to be returned

@return		The status of the query.
*/


status_t
sadict_query(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t		*value
);



/**
@brief		Used to insert data into the dictionary

@param		dictionary
				pointer to the dictionary that holds everything

@param		key
				a pointer to the key. the key is the index

@param		value
				the value to be inserted

@return		The status of the insertion.
*/

status_t
sadict_insert(
	dictionary_t 	*dictionary,
	ion_key_t 		key,
	ion_value_t		value
);


/**
@brief		Used to create the dictionary and get the ball rolling

@param		key_type
				the type of the key

@param		key_size
				the size that all the keys will be

@param		value_size
				the size that all the values will hold

@param		dictionary_size
				the size of the static array

@param		handler
				the pointer to the handler that goes with the dictionary
@param		dictionary
				the pointer to the dictionary that will be returned to the user

@return		to be filled in.
*/

status_t
sadict_create(
		key_type_t				key_type,
		int 					key_size,
		int 					value_size,
		int 					dictionary_size,
		dictionary_handler_t 	*handler,
		dictionary_t 			*dictionary
);


/**
@brief		Used to delete data from the dictionary given a key

@param		dictionary
				pointer to the dictionary that holds everything

@param		key
				a pointer to the key. the key is the index

@return		The status of the deletion.
*/
status_t
sadict_delete(
		dictionary_t 	*dictionary,
		ion_key_t 		key
);


/**
@brief		Used to delete the entire dictionary

@param		dictionary
				pointer to the dictionary that holds everything

@return		The status of the deletion.
*/

status_t
sadict_destroy(
		dictionary_t 	*dictionary
);


/**
@brief		Used to update data from the dictionary given a key

@param		dictionary
				pointer to the dictionary that holds everything

@param		key
				a pointer to the key. the key is the index

@param		value
				a pointer to the value that will replace the value at the key location

@return		The status of the update.
*/

status_t
sadict_update(
		dictionary_t 	*dictionary,
		ion_key_t 		key,
		ion_value_t 	value
);


/**
@brief		Finds stuff, dunno how this works

@param		dictionary
 	 	 	 	 a pointer to the dictionary that holds everything

@param		pred
 	 	 	 	 unkown

@param		cursor
 	 	 	 unkown

@return	unknown
*/

status_t
sadict_find(
		dictionary_t			*dictionary,
		predicate_t				*pred,
		dict_cursor_t			*cursor
);
#endif /* SAHANDLER_H_ */

