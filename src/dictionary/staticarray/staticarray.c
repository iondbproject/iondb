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

long long
ipow(
	int base,
	int exp
)
{
	long long temp = 1;

	int x;

	for(x=0;x<exp;x++)
	{
		temp *=base;
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

	//checks for invalid key size
	if(key_size >4 || key_size <= 0)
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


printf("The array size %llu \n The bucket size %lu \n The value size %d \n The total size %llu \n",
			starray->array_size,sizeof(bucket_t),value_size, starray->array_size*(sizeof(bucket_t) + value_size));

printf("The key size is %d\n", starray->key_size);
printf("The max number of elements in the array is %llu\n\n", starray->maxelements);
fflush(stdout);
	//creates room on the heap for bucket array and char value array
	starray->array = malloc(starray->array_size*(sizeof(bucket_t) + value_size));

	long long x;

	for(x=0;x<array_size;x++){
		((bucket_t *)&(starray->array[x]))->status= EMPTY;
	}
	fflush(stdout);

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
			return err_item_not_found;
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
	long long m = starray->maxelements;

	//gets key_size
	int s = starray->key_size;

	//b points at the begining of the bucket array
	bucket_t *b = (bucket_t *) starray->array;

	//points at the beinging of the char array section
	char *value_start = ((char *) b) + m*sizeof(bucket_t);

	if(strlen(key)!=starray->key_size)
	{
		return status_incorrect_keysize;
	}

	//gets the integer version of the key
	long long kk = key_to_index(key, s);

	//special key to find the value location
	long long value_key = kk * starray->value_size;

	bucket_t *current = &b[kk];

	int value_length = strlen(value);

	printf("The status is %d\n",current->status);

	if(current->status==OCCUPIED || kk >= starray->array_size || kk <= 0)
	{
		return status_incorrect_key;
	}
	else if(value_length==0 || value_length >= starray->value_size)
	{
		return status_incorrect_value;
	}
	else
	{
		memcpy(&(value_start[value_key]), value,starray->value_size);
		current->value = &(value_start[value_key]);
		current->status = OCCUPIED;
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
