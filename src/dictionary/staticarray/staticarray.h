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

// used for the bucket to tell if it is holding something or not
#define EMPTY 0
#define OCCUPIED 1

long long
ipow(
	int base,
	int exp
);

/**
@brief		Struct used to hold the whole static array and global information
			The array is broken into two halfs. The first holding the bucket,
			the second holding the char array.
*/
typedef struct static_array
{
	int			key_size;		/**< Holds the key size  */
	int			value_size;		/**< Holds the value size  */
	long long	maxelements;	/**< Holds the maximum numbers of elements
	 	 	 	 	 	 	 	     that can be held*/
	long long	array_size;		/**< Holds the array size  */
	char 			(* compare)(ion_key_t, ion_key_t, ion_key_size_t);
	char		*array;			/**< points to the whole static array. */
	key_type_t	key_type;		/**< Holds the type of the key. */
} static_array_t;


/**
@brief		A struct that holds the availability of the location and a pointer to the char data
*/
typedef struct bucket
{
	char	status;				/**< availability. EMPTY or OCCUPIED */
	ion_value_t	value;				/**< pointer pointing to the char data */
} bucket_t;


/**
@brief		This function creates the dictionary

@param		starray
				Pointer to the static array that will be created
@param 		key_size
				The size of the key in bytes.
@param		value_size
				The size of the value in bytes.
@param		array_size
				the size of the array.
@return		The status describing the result of the creation.
*/
status_t
sa_dictionary_create(
		static_array_t			*starray,
		int 					key_size,
		int 					value_size,
		long long				array_size,
		char 			(* compare)(ion_key_t, ion_key_t, ion_key_size_t)
);

/**
@brief		This function that updates a value given a key

@param		starray
				Pointer to the data structure
@param 		key
				a pointer to the key
@param		value
				a pointer to the value to be stored

@return		The status describing the result of the update.
*/
status_t
sa_update(
		static_array_t			*starray,
		ion_key_t 				key,
		ion_value_t 			value
);

void
sa_find(); //to be made later


/**
@brief		This function that fetches a value given a key

@param		starray
				Pointer to the data structure
@param 		key
				a pointer to the key
@param		value
				the value is found and returned to the user through this double pointer

@return		The status describing the result of the getting of information.
*/
status_t
sa_get(
	static_array_t				*starray,
	ion_key_t 					key,
	ion_value_t 				*value
);


/**
@brief		This function that inserts a value given a key

@param		starray
				Pointer to the data structure
@param 		key
				a pointer to the key
@param		value
				a pointer to the value to be inserted into the static array

@return		The status describing the result of the insert
 */
status_t
sa_insert(
	static_array_t				*starray,
	ion_key_t 					key,
	ion_value_t 				value
);

/**
@brief		a function that returns a numerical representation of the key

@param		key
				a pointer to the key

@param		key_size
				the size of the key

@return	the numerical key to be treated as an index
*/

long long
key_to_index(
		ion_key_t				key,
		int						key_size
);


/**
@brief		a function that deletes a value given a key

@param		starray
				Pointer to the data structure
@param		key
				a pointer to the key

@return		The status describing the result of the delete
*/
status_t
sa_delete(
	static_array_t			*starray,
	ion_key_t				key
);

/**
@brief		a function that deletes the whole data structure

@param		starray
				Pointer to the data structure

@return 	The status describing the result of the deletion of the dictionary
*/

status_t
sa_destroy(
	static_array_t			*starray
);

#endif /* STATICARRAY_H_ */
