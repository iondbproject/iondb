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
	return handler->create_dictionary(key_type, key_size, value_size, dictionary_size, handler, dictionary);
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
	ion_value_t 				*value
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
dictionary_compare_value(
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
dictionary_compare_char_array(
	ion_key_t 		first_key,
	ion_key_t		second_key,
	ion_key_size_t	key_size
)
{
	return memcmp((char *)first_key, (char *)second_key, key_size);
}
