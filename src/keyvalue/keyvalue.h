/******************************************************************************/
/**
@file
@author		Graeme Douglas
@brief		User keyvalue store API (collection level).
*/
/******************************************************************************/

#include "./../dictionary/dicttypes.h"

typedef struct kv_collection
{
	dictionary_t	*primary_index;
	// TODO: List or of secondary indexes?
} kv_collection_t;

status_t
kv_create(
	char		*name,
	kv_collection_t	*collection,
	dictionary_t	*primary_index
);

status_t
kv_open(
	char		*name,
	kv_collection_t	*collection
);

status_t
kv_put(
	kv_collection_t	*collection,
	ion_key_t	key,
	ion_value_t		value
);

status_t
kv_get(
	kv_collection_t	*collection,
	ion_key_t	key,
	ion_value_t		*value
);

status_t
kv_update(
	kv_collection_t	*collection,
	ion_key_t	key,
	ion_value_t		value
);

// Potential future methods:
status_t
kv_add_secondary_index(
	kv_collection_t	*collection,
	dictionary	*secondary_index
);
