/******************************************************************************/
/**
@file 		staticarray.c
@author		Raffi Kudlac
@brief		A static array to store information
 */
/******************************************************************************/

#include "staticarray.h"
#include <stdlib.h>
#include <stdio.h>

unsigned long
ipow(
	int base,
	int exp
)
{

	unsigned long result = 1;
	while (exp)
	{
		if (exp & 1)
			result *= base;
		exp >>= 1;
		base *= base;
	}
printf("The result is %lu\n", result);
	return result;
}

#ifndef NULL
#define NULL ((void*) 0)
#endif

status_t
sa_dictionary_create(
		static_array_t 			*starray,
		int 					key_size,
		int 					value_size,
		long					array_size
)
{
printf("The key size right now is %d\n",key_size);
	//checks for invalid key size
	if(key_size >= 4 || key_size <= 0)
	{
		key_size = 4;
		starray->key_size=key_size;
		starray->maxelements = ipow(256,key_size);
	}
	else
	{
		starray->maxelements = ipow(256,key_size);
		starray->key_size = key_size;
	}
printf("The key size right now is %d\n",key_size);
printf("The size of the max emelents right now is %llu\n", starray->maxelements);

	//checks for invalid array size
	if(array_size <=0 || array_size > starray->maxelements)
	{
		starray->array_size = starray->maxelements;
	}
	else
	{
		starray->array_size = array_size;
	}

	//checks valid entry for value size
	if(value_size <=0 || value_size > 1000)
	{
		value_size = 1000;
		starray->value_size = value_size;
	}
	else
	{
		starray->value_size = value_size;
	}


printf("The array size %lu \n The bucket size %d \n The value size %d \n The total size %lu \n",
			starray->array_size,sizeof(bucket_t),value_size, starray->array_size*(sizeof(bucket_t) + value_size));

printf("The key size is %d\n", starray->key_size);
printf("The max number of elements is size is %lu\n\n", starray->maxelements);
fflush(stdout);
	//creates room on the heap for bucket array and char value array
	starray->array = malloc(starray->array_size*(sizeof(bucket_t) + value_size));

	int x;

	for(x=0;x<array_size;x++){
		((bucket_t *)&(starray->array[x]))->status= EMPTY;
	}

	return status_ok;
}

status_t
sa_update(
		static_array_t 			*starray,
		ion_key_t 				key,
		ion_value_t 			value
)
{
	//gets the maximum number of elements that could exist
	long m = starray->maxelements;

	//gets key_size
	int s = starray->key_size;

	//gets the index from the key
	long k = key_to_index(key, s);

	//b points at the begining of the bucket array
	bucket_t *b = (bucket_t *) starray->array;

	//points at the beinging of the char array section
	char *value_start = ((char *) b) + m*sizeof(bucket_t);

	if (k > ipow(256,s))
	{
		return status_duplicate_key;
	}
	else{
		//copies value to the location in the char array section
		memcpy(&(value_start[k]),value,starray->value_size);
		bucket_t current = b[k];

		if (current.status == EMPTY)
		{
			current.status = OCCUPIED;
		}
		return status_ok;
	}

}

void sa_find(){ //dont do this one right now

}

status_t
sa_get(
	static_array_t				*starray,
	ion_key_t 					key,
	ion_value_t 				*value
)
{

		int v_size = starray->value_size;

		*value = malloc(sizeof(v_size));

		//gets the maximum number of elements that could exist
		//long m = ((static_array_t *)(dictionary->instance))->maxelements;

		//gets key_size
		int s = starray->key_size;

		//gets the index from the key
		long k = key_to_index(key, s);

		//b points at the begining of the bucket array
		bucket_t *b = (bucket_t *) starray->array;

		char *value_start = b[k].value;

		if(b[k].status == EMPTY || k > pow(256,s))
		{
			return status_duplicate_key; //wrong return message but there is an error, no element found
		}
		else
		{
			*value = value_start;
			memcpy(value,value_start,v_size);
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
	//gets the maximum number of elements that could exist
	long m = starray->maxelements;

	//gets key_size
	int s = starray->key_size;

	//b points at the begining of the bucket array
	bucket_t *b = (bucket_t *) starray->array;

	//points at the beinging of the char array section
	char *value_start = ((char *) b) + m*sizeof(bucket_t);

	//gets the integer version of the key
	long k = key_to_index(key, s);

	bucket_t current = b[k];

	if(current.status==OCCUPIED || k > pow(256,s)){
		return status_duplicate_key;
	}
	else{
		memcpy(&(value_start[k]), &value,starray->value_size);
		current.value = value_start;
		current.status = OCCUPIED;
		return status_ok;
	}

}

int
key_to_index(
		ion_key_t key,
		int key_size
)
{
	int result = 0;
	memcpy((char *)&result, &key, key_size);

	 return result*key_size;
}

status_t
sa_delete(
	static_array_t		*starray,
	ion_key_t			key
)
{

	//gets the maximum number of elements that could exist
	long m = starray->maxelements;

	//gets key_size
	int s = starray->key_size;

	//b points at the begining of the bucket array
	bucket_t *b = (bucket_t *) starray->array;

	//points at the beinging of the char array section
	char *value_start = ((char *) b) + m*sizeof(bucket_t);

	//gets the integer version of the key
	long k = key_to_index(key, s);

	if(b[k].status == EMPTY || k > pow(256,s))
	{
		return status_duplicate_key;
	}
	else
	{
		b[k].status = EMPTY;
		memcpy(&(value_start[k]), "",starray->value_size);
		return status_ok;
	}
}


status_t
sa_destroy(
	static_array_t		*starray
)
{
	//b points at the begining of the bucket array
	bucket_t *b = (bucket_t *) starray->array;

	free(b);

	return status_ok;

}
