/******************************************************************************/
/**
@file		keyvalue.h
@author		Graeme Douglas
@brief		A key-value store API.
*/
/******************************************************************************/

#ifndef KEYVALUE_H
#define KEYVALUE_H

#ifdef __cplusplus
extern "C" {
#endif

struct collection_handler
{
	dictionary_t	dictionary;	/**< The dictionary handler to use
					     for storing data in. */
};
typedef struct collection_handler collection_handler_t;

// TODO: Finish designing this function.
/**
@brief		Create a new collection.
@details	The collection will be created intelligently, choosing the
		correct underlying data structures and algorithms according
		to the need.
@todo		Maybe include a forcing option to force use a specific type
		of dictionary?
*/
status_t
kv_create(
	char			*name,
	length_t		keysize,
	length_t		valuesize,
	storage_type_t		storagetype,
	bool_t			allowduplicates
);

/**
@brief		Remove a collection from the database.
*/
status_t
kv_destroy(
	char			*name
);

/**
@brief		Open a collection.
@param		name
			The name of the collection to open.
@returns	A collection handler instance for the collection named
		@p name.
*/
collection_handler_t
kv_open(
	char			*name
)

/**
@brief		Add a value to a collection given a key.
@param		handler
			A pointer to the collection handler instance
			to insert a value to.
@param		key
			The key to associate @p value with. 
@param		value
			The value to add.
@return		A status describing the result of the call.
*/
status_t
kb_put(
	collection_handler_t	*handler,
	key_t			key,
	value_t			value
);

/**
@brief		Get the values associated with a given key.
@param		handler
			A pointer to the collection handler instance
			to retrieve a value from.
@param		key
			The key to get the values the value for.
@param		value
			A pointer to a value variable to write to.
@return		An iterator that can iterate over the values associated with @p key.
*/
kv_iterator_t
kv_get(
	collection_handler_t	*handler,
	key_t			key,
	value_t			*value
);

/**
@param		handler
			A pointer to the collection handler instance
			to update.
@param		key
			The key to associate @p value with.
@param		value
			The replacement value.
@return		A status describing the result of the call.
*/
status_t
kv_update(
	collection_handler_t	*handler,
	key_t			*key
);

/**
@param		handler
			A pointer to the collection handler instance
			to update.
@param		key
			The key whose associated values are to be deleted.
@return		A status describing the result of the call.
*/
status_t
kv_delete_all(
	collection_handler_t	*handler,
	key_t			*key
);

#ifdef __cplusplus
}
#endif

#endif
