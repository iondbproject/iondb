/******************************************************************************/
/**
@file
@author		Graeme Douglas, Scott Fazackerley
@see		For more information, refer to @ref dictionary.c.
*/
/******************************************************************************/

#include "dictionary.h"


// creates a named instance of a database
status_t
dictionary_create(
		dictionary_handler_t 	*handler,
		dictionary_t 			*dictionary,
		key_type_t				key_type,
		int 					key_size,
		int 					value_size,
		int 					dictionary_size
)
{
	//using the handler, create the dictionary
	/*	//register the type of key being used by the dictionary (Important for comparison op)
		dictionary->key_type = key_type;*/
	char (* compare)(ion_key_t, ion_key_t, ion_key_size_t);

	switch (key_type)
			{
				case key_type_numeric_signed:
				{
					compare = dictionary_compare_signed_value;
					break;
				}
				case key_type_numeric_unsigned:
				{
					compare = dictionary_compare_unsigned_value;
					break;
				}
				case key_type_char_array:
				{
					compare = dictionary_compare_char_array;
					break;
				}
				default:
				{
					//do something - you must bind the correct comparison function
					break;
				}
			}

	return handler->create_dictionary(key_type, key_size, value_size, dictionary_size, compare, handler, dictionary);

}

//inserts a record into the dictionary
//each dictionary will have a specific handler?
status_t
dictionary_insert(
	/*dictionary_handler_t	*handler,*/
	dictionary_t 				*dictionary,
	ion_key_t 					key,
	ion_value_t 				value)
{
	return dictionary->handler->insert(dictionary, key, value);
}

status_t
dictionary_get(
	dictionary_t 				*dictionary,
	ion_key_t 					key,
	ion_value_t 				value
)
{
	return dictionary->handler->get(dictionary, key, value);
}
status_t
dictionary_update(
		dictionary_t 			*dictionary,
		ion_key_t 				key,
		ion_value_t 			value)
{
	return dictionary->handler->update(dictionary, key, value);
}

status_t
dictionary_delete_dictionary(
	dictionary_t		*dictionary
)
{
	return dictionary->handler->delete_dictionary(dictionary);
}

status_t
dictionary_delete(
	dictionary_t		*dictionary,
	ion_key_t			key
)
{
	return dictionary->handler->delete(dictionary,key);
}

char
dictionary_compare_unsigned_value(
	ion_key_t 		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
	)
{
	int idx;
	char return_value;
	/**
	 * In this case, the endiannes of the process does matter as the code does
	 * a direct comparison of bytes in memory starting for MSB.
	 */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	for (idx = key_size -1; idx >= 0; idx--)
	{
		if ((return_value = ((*(first_key+idx) > *(second_key+idx)) - (*(first_key+idx) < *(second_key+idx)))) != ZERO)
		{
			return return_value;
		}
	}
	return return_value;
#else
	/** @TODO This is a potential issue and needs to be tested on SAMD3 */
	for (idx = 0 ; idx < key_size ; idx++)
	{
		if ((return_value = ((*(first_key+idx) > *(second_key+idx)) - (*(first_key+idx) < *(second_key+idx)))) != ZERO)
		{
			return return_value;
		}
	}
	return return_value;
#endif
}

char
dictionary_compare_signed_value(
	ion_key_t 		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
	)
{
	int idx;
	char return_value;

	/**
	 * In this case, the endiannes of the process does matter as the code does
	 * a direct comparison of bytes in memory starting for MSB.
	 */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	//check the MSByte as signed
	idx = key_size - 1;						//Start at the MSB
#if (DEBUG == 1)
	printf("key 1: %i key 2: %i \n",*(char *)(first_key+idx),*(char *)(second_key+idx));
#endif
	if ((return_value = ((*(char *)(first_key+idx) > *(char *)(second_key+idx)) - (*(char *)(first_key+idx) < *(char *)(second_key+idx)))) != ZERO)
		{
			return return_value;
		}
	//and then the rest as unsigned
	for (idx = key_size - 2; idx >= 0; idx--)
	{
		if ((return_value = ((*(first_key+idx) > *(second_key+idx)) - (*(first_key+idx) < *(second_key+idx)))) != ZERO)
		{
			return return_value;
		}
	}
	return return_value;
#else
	/** @TODO This is a potential issue and needs to be tested on SAMD3 */
	for (idx = 0 ; idx < key_size ; idx++)
	{
		if ((return_value = ((*(first_key+idx) > *(second_key+idx)) - (*(first_key+idx) < *(second_key+idx)))) != ZERO)
		{
			return return_value;
		}
	}
	return return_value;
#endif
}


char
dictionary_compare_char_array(
	ion_key_t 		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
)
{
	return memcmp((char *)first_key, (char *)second_key, key_size);
}

err_t
dictionary_build_predicate(
	dictionary_t			*dictionary,
	predicate_t				**predicate,
	predicate_operator_t	type,
	ion_key_t				key,
	...
)
{
	va_list arg_list;

	va_start (arg_list, key);         // Initialize the argument list.

	(*predicate) = (predicate_t *)malloc(sizeof(predicate_t));

	(*predicate)->type = type;

	ion_key_size_t key_size = ((dictionary_parent_t *)dictionary->instance)->record.key_size;

	switch(type)
	{
		case predicate_equality:
		{
			(*predicate)->statement.equality.equality_value = (ion_key_t) malloc (key_size);
			memcpy((*predicate)->statement.equality.equality_value, key, key_size);
			(*predicate)->destroy = dictonary_destroy_predicate_statement;
			break;
		}
		case predicate_range:
		{
			if (((*predicate)->statement.range.geq_value = (ion_key_t) malloc (key_size)) == NULL)
			{
				return err_out_of_memory;
			}
			if(((*predicate)->statement.range.leq_value = (ion_key_t) malloc (key_size)) == NULL)
			{
				free((*predicate)->statement.range.geq_value);
				return err_out_of_memory;
			}
			(*predicate)->destroy = dictonary_destroy_predicate_range;
			//copy in first value
			memcpy((*predicate)->statement.range.geq_value, key, key_size);
			//extract optional parameter
			va_start(arg_list,key);
			//and extract the second key from the list
			memcpy((*predicate)->statement.range.geq_value,	 (ion_key_t)(va_arg(arg_list,ion_key_t)), key_size);
			va_end(arg_list);
			break;
		}
		/*case predicate_predicate:
		{
			break;
		}*/
		default:
		{
			return err_invalid_predicate;
			break;
		}
	}
	return err_ok;
}

void
dictonary_destroy_predicate_statement(
	predicate_t		**predicate
)
{
	free((*predicate)->statement.equality.equality_value);
	free(*predicate);
	*predicate = NULL;

}

void
dictonary_destroy_predicate_range(
	predicate_t		**predicate
)
{
	free((*predicate)->statement.range.geq_value);
	free((*predicate)->statement.range.leq_value);
	free(*predicate);
		*predicate = NULL;
}
