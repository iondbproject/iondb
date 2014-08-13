/******************************************************************************/
/**
@file 		staticarray.c
@author		Raffi Kudlac
@brief		A static array to store information
 */
/******************************************************************************/

#include "staticarray.h"

//used to calculate max size of array
sa_max_size_t
ipow(
	int base,
	int exp
)
{
	sa_max_size_t temp = 1;

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
		ion_key_size_t 			key_size,
		ion_value_size_t 		value_size,
		sa_max_size_t			array_size,
		char					(*compare)(ion_key_t, ion_key_t, ion_key_size_t)
)
{

	// checks for invalid key size
	if (key_size > 3 || key_size <= 0)
	{
		return status_key_out_of_bounds;

	}
	else
	{
		starray->maxelements = ipow(256, key_size);
		starray->key_size 	 = key_size;
	}

	// checks for invalid array size
	if(array_size <= 0 || array_size > starray->maxelements)
	{
		return status_array_size_out_of_bounds;
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
	starray->array = calloc(starray->array_size,
			(sizeof(bucket_t) + value_size));

	int j = 0;
	//makes pointer pointing at first bucket location
	bucket_t *bb = (bucket_t *) starray->array;

	// points at the beinging of the char array section
	ion_value_t value_start = ((ion_value_t) bb) +
			starray->array_size * sizeof(bucket_t);

	//makes all pointer point to locations in the char array section
	//this is to reduce code in other functions
	for(j = 0; j < starray->array_size; j++)
	{
		bb[j].value = &value_start[j*starray->value_size];
	}
	starray->compare = compare;
	return status_ok;
}


status_t
sa_update(
		static_array_t 			*starray,
		ion_key_t 				key,
		ion_value_t 			value
)
{
	bucket_t *b 	= NULL;
	status_t sat 	= find_bucket(&b, starray,key);

	if (sat != status_ok)
	{
		return 	sat;
	}
	else
	{
		// copies value to the location in the char array section
		memcpy(b->value, value, starray->value_size);

		if (b->status == EMPTY)
		{
			b->status = OCCUPIED;
		}
		return status_ok;
	}
}

//all taken care of in the handler
void
sa_find()
{

}


status_t
sa_get(
	static_array_t				*starray,
	ion_key_t 					key,
	ion_value_t 				*value
)
{
	int v_size 	= starray->value_size;
	*value 	   	= malloc(sizeof(v_size));
	bucket_t *b = NULL;
	status_t s 	= find_bucket(&b, starray,key);

	if (s != status_ok)
	{
		return s;
	}
	else if (b->status == EMPTY)
	{
		return status_empty_slot;
	}
	else
	{
		strcpy((char*)*value, (char*)b->value);
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
	bucket_t *b = NULL;
	status_t s 	= find_bucket(&b, starray,key);

	if (s != status_ok)
	{
		return s;
	}
	else if(b->status == OCCUPIED)
	{
		return status_occupied;
	}
	else
	{
		memcpy(b->value, value, starray->value_size);
		b->status = OCCUPIED;
		return status_ok;
	}
}


sa_max_size_t
key_to_index(
		ion_key_t 		key,
		ion_key_size_t 	key_size
)
{
	sa_max_size_t result = 0;
	memcpy((char *)&result, key, key_size);

	return result;
}


status_t
sa_delete(
	static_array_t		*starray,
	ion_key_t			key
)
{
	bucket_t *b = NULL;
	status_t s 	= find_bucket(&b, starray,key);

	if (s != status_ok)
	{
		return 	s;
	}
	else if(b->status == EMPTY)
	{
		return status_empty_slot;
	}
	else
	{
		b->status 	= EMPTY;
		memcpy(b->value, "", starray->value_size);
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


status_t
find_bucket(
	bucket_t 		**b,
	static_array_t	*starray,
	ion_key_t 		key
)
{
	int s 			= starray->key_size;
	sa_max_size_t k = key_to_index(key, s);

	if (k >= starray->array_size || k < 0)
	{
		return status_key_out_of_bounds;
	}

	bucket_t *bb 	= (bucket_t *) starray->array;
	*b 				= &bb[k];

	return status_ok;
}
