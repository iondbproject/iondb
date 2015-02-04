/******************************************************************************/
/**
@file
@author		Graeme Douglas
@brief		User keyvalue store API (collection level).
*/
/******************************************************************************/

#include "./../dictionary/dicttypes.h"

/**
 @brief Struct to maintain instance data for a given collection.
 */
typedef struct kv_collection
{
	dictionary_t		*primary_index;			/**< */
	// TODO: List or of secondary indexes?
} kv_collection_t;

/**

 @param name
 @param collection
 @param primary_index
 @return
 */
status_t
kv_create(
	char				*name,
	kv_collection_t		*collection,
	dictionary_t		*primary_index
);

/**

 @param name
 @param collection
 @return
 */
status_t
kv_open(
	char				*name,
	kv_collection_t		*collection
);

/**

 @param collection
 @param key
 @param value
 @return
 */
status_t
kv_put(
	kv_collection_t		*collection,
	ion_key_t			key,
	ion_value_t			value
);

/**

 @param collection
 @param key
 @param value
 @return
 */
status_t
kv_get(
	kv_collection_t		*collection,
	ion_key_t			key,
	ion_value_t			*value
);

/**

 @param collection
 @param key
 @param value
 @return
 */
status_t
kv_update(
	kv_collection_t		*collection,
	ion_key_t			key,
	ion_value_t			value
);


/**

 @param collection
 @param secondary_index
 @return
@todo Potential future methods:
 */
status_t
kv_add_secondary_index(
	kv_collection_t	*collection,
	dictionary	*secondary_index
);
