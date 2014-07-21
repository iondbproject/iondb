/******************************************************************************/
/**
@file 		staticarray.c
@author		Raffi Kudlac
@brief		A static array to store information
 */
/******************************************************************************/

#include "staticarray.h"

long long
ipow(
	int base,
	int exp
)
{
	long long temp = 1;

	int x;

	for (x = 0; x < exp; x++)
	{
		temp *= base;
	}

	return temp;
}

#ifndef NULL
#define NULL ((void*) 0)
#endif

status_t
sa_dictionary_create(
		static_array_t 			*starray,
		int 					key_size,
		int 					value_size,
		long long				array_size
)
{

	// checks for invalid key size
	if (key_size > 3 || key_size <= 0)
	{
		return status_incorrect_keysize;

	}
	else
	{
		starray->maxelements = ipow(256, key_size);
		starray->key_size 	 = key_size;
	}


	// checks for invalid array size
	if(array_size <= 0 || array_size > starray->maxelements)
	{
		starray->array_size  = starray->maxelements;
	}
	else
	{
		starray->array_size  = array_size;
	}

	// checks valid entry for value size
	if (value_size <= 0)
	{
		return status_incorrect_value;
	}
	else
	{
		starray->value_size = value_size;
	}

	// creates room on the heap for bucket array and char value array
	starray->array = calloc(
						starray->array_size,
						(sizeof(bucket_t) + value_size)
					);

	return status_ok;
}

status_t
sa_update(
		static_array_t 			*starray,
		ion_key_t 				key,
		ion_value_t 			value
)
{
	// gets the maximum number of elements that could exist
	long m = starray->maxelements;

	// gets key_size
	int s = starray->key_size;

	// gets the index from the key
	long k = key_to_index(key, s);

	long long value_key = k * starray->value_size;

	// b points at the begining of the bucket array
	bucket_t *b = (bucket_t *) starray->array;

	// points at the beinging of the char array section
	ion_value_t value_start = ((ion_value_t) b) + m * sizeof(bucket_t);

	if (k >= starray->array_size || k < 0)
	{
		return 	status_incorrect_keysize;
	}
	else
	{
		// copies value to the location in the char array section
		memcpy(&(value_start[value_key]), value,starray->value_size);
		bucket_t *current 	= &b[k];

		if (current->status == EMPTY)
		{
			current->value 	= &(value_start[value_key]);
			current->status = OCCUPIED;
		}
		return status_ok;
	}

}

void
sa_find()
{ //dont do this one right now

}

status_t
sa_get(
	static_array_t				*starray,
	ion_key_t 					key,
	ion_value_t 				*value
)
{

		int v_size = starray->value_size;

		*value 	   = malloc(sizeof(v_size));

		// gets key_size
		int s = starray->key_size;

		// gets the index from the key
		long k = key_to_index(key, s);

		// b points at the begining of the bucket array
		bucket_t *b = (bucket_t *) starray->array;

		// char *value_start = b[k].value;

		if (k >= starray->array_size || k < 0)
		{
			return status_incorrect_keysize;
		}
		else if (b[k].status == EMPTY)
		{
			return status_empty_slot;
		}
		else
		{
			strcpy((char*)*value, (char*)b[k].value);
			return status_ok;
		}
}

status_t
sa_insert(
	static_array_t				*starray,
	ion_key_t 					key,
	ion_value_t 				value
)
{
	// gets the maximum number of elements that could exist
	long long m = starray->maxelements;

	// gets key_size
	int s				= starray->key_size;

	// b points at the begining of the bucket array
	bucket_t *b			= (bucket_t *) starray->array;

	// points at the beinging of the char array section
	ion_value_t value_start	= ((ion_value_t) b) + m*sizeof(bucket_t);


	// gets the integer version of the key
	long long kk		= key_to_index(key, s);

	//special key to find the value location
	long long value_key = kk * starray->value_size;

	bucket_t *current 	= &b[kk];

	if (current->status == OCCUPIED)
	{
		return status_occupied;
	}
	else if (kk >= starray->array_size || kk <= 0)
	{
		return status_incorrect_keysize;
	}
	else
	{
		memcpy(&(value_start[value_key]), value, starray->value_size);
		current->value	= &(value_start[value_key]);
		current->status	= OCCUPIED;
		return status_ok;
	}
}

long long
key_to_index(
		ion_key_t key,
		int key_size
)
{
	long long result = 0;
	memcpy((char *)&result, key, key_size);

	return result;
}

status_t
sa_delete(
	static_array_t		*starray,
	ion_key_t			key
)
{

	// gets the maximum number of elements that could exist
	long m				= starray->maxelements;

	// gets key_size
	int s 				= starray->key_size;

	// b points at the begining of the bucket array
	bucket_t *b			= (bucket_t *) starray->array;

	// points at the beinging of the char array section
	char *value_start	= ((char *) b) + m*sizeof(bucket_t);

	// gets the integer version of the key
	long k				= key_to_index(key, s);

	if (k >= starray->array_size || k<0)
	{
		return 	status_incorrect_keysize;
	}
	else if(b[k].status == EMPTY)
	{
		return status_empty_slot;
	}
	else
	{
		b[k].status 	= EMPTY;
		memcpy(&(value_start[k]), "", starray->value_size);
		return status_ok;
	}
}

status_t
sa_destroy(
	static_array_t		*starray
)
{
	//b points at the begining of the bucket array
	bucket_t *b 	= (bucket_t *) starray->array;

	free(b);
	starray->array	= NULL;

	return status_ok;
}

char
dictionary_compare_value(
	ion_key_t 		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
)
{
	int k1 key_to_index(first_key, key_size);
	int k2 key_to_index(second_key, key_size);

	if(k1 == k2)
	{
		return "0";
	}
	else if(k1 > k2)
	{
		return "1";
	}
	else
	{
		return "2";
	}
}


}
