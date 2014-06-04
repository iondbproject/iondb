/**
 *
 */

#ifndef OADICTIONARYHANDLER_H_
#define OADICTIONARYHANDLER_H_

#include "./../dicttypes.h"
#include "./../dictionary.h"
#include "./../../kv_system.h"
#include "oahash.h"

/**
 *
 */
struct oadict_cursor
{
	dict_cursor_t		super;		//Super type of this cursor
	hash_t	first;					//First visited spot
	hash_t	current;				//Currently visited spot
}oadict_cursor_t;

//this will register the handler for the dictionary
err_t
oadict_init(dictionary_handler_t * handler);

err_t
oadict_insert(
	dictionary_t * dictionary,
	key_t * key,
	value_t * value);

err_t
oadict_query(
	dictionary_t *dictionary,
	key_t *key,
	value_t	**value);

err_t
oadict_create_dictionary(
		int key_size,
		int value_size,
		int dictionary_size,
		dictionary_handler_t * handler,
		dictionary_t * dictionary);

/**
 *
 * @param dictionary
 * @param key
 * @return
 */
err_t
oadict_delete(
		dictionary_t * dictionary,
		key_t * key);

/**
 *
 * @param dictionary
 * @return
 */
err_t
oadict_delete_dictionary(
		dictionary_t * dictionary);
/**
 *
 * @param dictionary
 * @param key
 * @param value
 * @return
 */
err_t
oadict_update(
		dictionary_t * dictionary,
		key_t * key,
		value_t * value);

/**
 * @details Generates a cursor that contains
 * @param dictionary
 * @param cursor
 * @param key
 * @return
 */
err_t
oadict_find(
		dictionary_t * dictionary,
		predicate_t * predicate,
		dict_cursor_t * cursor);


#endif /* OADICTIONARYHANDLER_H_ */
