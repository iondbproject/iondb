/******************************************************************************/
/**
@file 		staticarray.h
@author		Raffi Kudlac
@brief		The .h file for the staticarray.c
			holds the static_array struct as well as the bucket struct
 */
/******************************************************************************/
#ifndef STATICARRAY_H_
#define STATICARRAY_H_

#include "./../../kv_system.h"
#include "./../dictionary.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//used for the bucket to tell if it is holding something or not
#define EMPTY 0
#define OCCUPIED 1

/**
@brief		Struct used to hold the whole static array and global information
*			The array is broken into two halfs. The first holding the bucket,
*			the second holding the char array.
*/
struct static_array{
	int key_size;
	int value_size;
	long maxelements;
	char *array;		//points to the whole static array.
};
typedef struct static_array static_array_t;


/**
 @brief		A struct that holds the availability of the location and a pointer to the char data
 */
struct bucket
{
	char status;	//availability. EMPTY or OCCUPIED
	char *value;	//pointer pointing to the char data
};
typedef struct bucket bucket_t;


/**
@brief		This function creates the dictionary

@param		*handker
				Pointer to the handler that is responsible for this dictionary
@param		*dictionary
				the dictionary that everything is stored in
@param 		key_size
				The size of the key in bytes.
@param		value_size
				The size of the value in bytes.
@param		array_size
				the size of the array.
@return		The status describing the result of the creation.
 */
status_t
sa_create(
		dictionary_handler_t 	*handler,
		dictionary_t 			*dictionary,
		int 					key_size,
		int 					value_size,
		int 					array_size
);

/**
@brief		This function that updates a value given a key

@param		*dictionary
				Pointer to the dictionary where everything is located
@param 		*key
				a pointer to the key
@param		value
				the value to be stored

@return		The status describing the result of the update.
 */
status_t
sa_update(
		dictionary_t 			*dictionary,
		ion_key_t 				key,
		ion_value_t 			value
);

void sa_find(); //to be made later


/**
@brief		This function that fetches a value given a key

@param		*dictionary
				Pointer to the dictionary where everything is located
@param 		*key
				a pointer to the key
@param		**value
				the value is found and returned to the user through this double pointer

@return		The status describing the result of the getting of information.
 */
status_t
sa_get(
	dictionary_t 				*dictionary,
	ion_key_t 					key,
	ion_value_t 				*value
);


/**
@brief		This function that inserts a value given a key

@param		*dictionary
				Pointer to the dictionary where everything is located
@param 		*key
				a pointer to the key
@param		*value
				the value to be inserted into the static array

@return		The status describing the result of the insert
 */
status_t
sa_insert(
	dictionary_t 				*dictionary,
	ion_key_t 					key,
	ion_value_t 				value
);

/**
 @brief		a function that returns a numerical representation of the key

 @param		*key
 	 	 	 	 a pointer to the key

 @param		key_size
 	 	 	 	 the size of the key

 @return	the numerical key to be treated as an index
 */

int
key_to_index(
		ion_key_t key,
		int key_size
);


/**
 @brief		a function that deletes a value given a key

 @param		*dictionary
				Pointer to the dictionary where everything is located
 @param		*key
 	 	 	 	 a pointer to the key

 @return	The status describing the result of the delete
 */
status_t
sa_delete(
	dictionary_t		*dictionary,
	ion_key_t			key
);

/**
 @brief		a function that deletes the whole dictionary

 @param		*dictionary
 	 	 	 	 a pointer to the dictionary to be destroyed

 @return 	The status describing the result of the deletion of the dictionary
 */

status_t
sa_destroy(
	dictionary_t		*dictionary
);

#endif /* STATICARRAY_H_ */
