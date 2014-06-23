/******************************************************************************/
/**
@file 		staticarray.c
@author		Raffi Kudlac
@brief		A static array to store information
 */
/******************************************************************************/

#include "staticarray.h"
#include <stdlib.h>

#define NULL ((void*) 0)

status_t
sa_dictionary_create(
		dictionary_handler_t 	*handler,
		dictionary_t 			*dictionary,
		int 					key_size,
		int 					value_size,
		int 					array_size
)
{

	static_array_t *st;

	st =(static_array_t *) malloc(sizeof(static_array_t));
	st->maxelements = pow(256,key_size);
	st->key_size = key_size;
	st->value_size = value_size;

	//creates room on the heap for bucket array and char value array
	st->array = malloc(st->maxelements*(sizeof(bucket_t) + value_size));//check with graeme about this line

	int x;

	for(x=0;x<array_size;x++){
		((bucket_t *)st->array[x])->status= EMPTY;
	}
	dictionary->instance = st;
	dictionary->handler = handler;

	return status_ok;
}

status_t
sa_update(
		dictionary_t 			*dictionary,
		ion_key_t 				key,
		ion_value_t 			value
)
{
	//gets the maximum number of elements that could exist
	long m = ((static_array_t *)(dictionary->instance))->maxelements;

	//gets key_size
	int s = ((static_array_t*)(dictionary->instance))->key_size;

	//gets the index from the key
	long k = key_to_index(key, s);

	//b points at the begining of the bucket array
	bucket_t *b = (bucket_t *) ((static_array_t*)(dictionary->instance))->array;

	//points at the beinging of the char array section
	char *value_start = ((char *) b) + m*sizeof(bucket_t);

	if (k > pow(256,s))
	{
		return status_duplicate_key;
	}
	else{
		//copies value to the location in the char array section
		memcpy(&(value_start[k]),value,
					((static_array_t*)(dictionary->instance))->value_size);
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
	dictionary_t 				*dictionary,
	ion_key_t 					key,
	ion_value_t 				*value
)
{

		int v_size = ((static_array_t*)dictionary->instance)->value_size;

		*value = malloc(sizeof(v_size));

		//gets the maximum number of elements that could exist
		//long m = ((static_array_t *)(dictionary->instance))->maxelements;

		//gets key_size
		int s = ((static_array_t*)(dictionary->instance))->key_size;

		//gets the index from the key
		long k = key_to_index(key, s);

		//b points at the begining of the bucket array
		bucket_t *b = (bucket_t *) ((static_array_t*)(dictionary->instance))->array;

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
	dictionary_t 				*dictionary,
	ion_key_t 					key,
	ion_value_t 				value
)
{
	//gets the maximum number of elements that could exist
	long m = ((static_array_t *)(dictionary->instance))->maxelements;

	//gets key_size
	int s = ((static_array_t*)(dictionary->instance))->key_size;

	//b points at the begining of the bucket array
	bucket_t *b = (bucket_t *) ((static_array_t*)(dictionary->instance))->array;

	//points at the beinging of the char array section
	char *value_start = ((char *) b) + m*sizeof(bucket_t);

	//gets the integer version of the key
	long k = key_to_index(key, s);

	bucket_t current = b[k];

	if(current.status==OCCUPIED || k > pow(256,s)){
		return status_duplicate_key;
	}
	else{
		memcpy(&(value_start[k]), &value,
				((static_array_t*)(dictionary->instance))->value_size);
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
	dictionary_t		*dictionary,
	ion_key_t			key
)
{

	//gets the maximum number of elements that could exist
	long m = ((static_array_t *)(dictionary->instance))->maxelements;

	//gets key_size
	int s = ((static_array_t*)(dictionary->instance))->key_size;

	//b points at the begining of the bucket array
	bucket_t *b = (bucket_t *) ((static_array_t*)(dictionary->instance))->array;

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
		memcpy(&(value_start[k]), "",
						((static_array_t*)(dictionary->instance))->value_size);
		return status_ok;
	}
}


status_t
sa_destroy(
	dictionary_t		*dictionary
)
{
	//b points at the begining of the bucket array
	bucket_t *b = (bucket_t *) ((static_array_t*)(dictionary->instance))->array;

	free(b);

	free(dictionary->instance);
	dictionary->instance = NULL;					// When releasing memory, set pointer to NULL

	return status_ok;

}
