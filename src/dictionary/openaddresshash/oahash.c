/******************************************************************************/
/**
@file		oahash.c
@author		Scott Ronald Fazackerley
@brief		Open Address Hash Map
@details	The open address hash map allows non-colliding entries into a hash table

@todo 	capture size of map
@todo 	prevent duplicate insertions
@todo  	When creating the hash-map, need to know something about what is going in it.
 		What we need to know if the the size of the key and the size of the data.
 		That is all.  Nothing else.
 */
/******************************************************************************/

#include "oahash.h"

err_t
oah_initialize(
    hashmap_t 	*hashmap,
    hash_t 		(*hashing_function)(hashmap_t *, char *, int),
    int 		key_size,
    int 		value_size,
    int 		size
)
{
	int i;
	hashmap->write_concern 		= wc_insert_unique;			/* By default allow unique inserts only */
	hashmap->record.key_size 	= key_size;
	hashmap->record.value_size 	= value_size;

	/* The hash map is allocated as a single contiguous array*/
	hashmap->map_size 			= size;
	hashmap->entry = (void *)malloc(
	        (hashmap->record.key_size + hashmap->record.value_size + 1)
	                * hashmap->map_size);
	hashmap->compute_hash 		= (*hashing_function);		/* Allows for binding of different hash functions
																depending on requirements */

#ifdef DEBUG
	io_printf("Record key size: %i\n", hashmap->record.key_size);
	io_printf("Record value size: %i\n", hashmap->record.value_size);
	io_printf("Size of map (in bytes): %i\n",
	        (hashmap->record.key_size + hashmap->record.value_size + 1)
	                * hashmap->map_size);
#endif

	if (hashmap->entry == NULL)
		return 1;

#ifdef DEBUG
	io_printf("Initializing hash table\n");
#endif

	/* Initialize hash table */
	for (i = 0; i < size; i++)
	{
		((hash_bucket_t *)(hashmap->entry
		        + ((hashmap->record.key_size + hashmap->record.value_size
		                + SIZEOF(STATUS)) * i)))->status = EMPTY;
	}
	return 0;
}

err_t
oah_destroy(
	hashmap_t 	*hash_map
)
{
	hash_map->compute_hash 		= NULL;
	hash_map->map_size 			= 0;
	hash_map->record.key_size 	= 0;
	hash_map->record.value_size	= 0;
	if (hash_map->entry != NULL)			//check to ensure that you are not freeing something already free
	{
		free(hash_map->entry);
		hash_map->entry = NULL;				//
		return err_ok;
	}
	else
	{
		return err_colllection_destruction_error;
	}
}

int
oah_get_location(
	hash_t 		num,
	int 		size
)
{
	return num % size;
}

err_t
oah_update(
	hashmap_t	 	*hash_map,
	ion_key_t 		key,
	ion_value_t		value
)
{
	//TODO: lock potentially required
	write_concern_t current_write_concern 	= hash_map->write_concern;
	hash_map->write_concern 				= wc_update;			//change write concern to allow update
	err_t result 							= oah_insert(hash_map, key, value);
	hash_map->write_concern 				= current_write_concern;
	return result;
}

err_t
oah_insert(
	hashmap_t 		*hash_map,
	ion_key_t 		key,
	ion_value_t 	value
)
{
	int i;

#ifdef DEBUG
	for (i = 0; i < hash_map->record.key_size; i++)
	{
		io_printf("%x ", key[i]);
	}
	io_printf("\n");
#endif

	hash_t hash 	= hash_map->compute_hash(hash_map, key,
							hash_map->record.key_size);			//compute hash value for given key

#ifdef DEBUG
	io_printf("hash value is %i\n", hash);
	io_printf("data value is ");
	for (i = 0; i < hash_map->record.value_size; i++)
	{
		io_printf("%c", *(value + i));
	}
	io_printf("\n");
#endif

	int loc			= oah_get_location(hash, hash_map->map_size);

#ifdef DEBUG
	io_printf("location %i\n", loc);
#endif

	// Scan until find an empty location - oah_insert if found
	int count 		= 0;

	hash_bucket_t *item;

	while (count != hash_map->map_size)
	{
		item 		= ((hash_bucket_t *)((hash_map->entry
		        			+ (hash_map->record.key_size + hash_map->record.value_size
		        				+ SIZEOF(STATUS)) * loc)));

		if (item->status == IN_USE) 	//if a cell is in use, need to key to
		{
			if (memcmp(item->data, key, hash_map->record.key_size) == IS_EQUAL)
			{
				if (hash_map->write_concern == wc_insert_unique)//allow unique entries only
				{
					return err_duplicate_key;
				}
				else if (hash_map->write_concern == wc_update)//allows for values to be updated											//
				{
					memcpy(item->data + hash_map->record.key_size, value,
					        (hash_map->record.value_size));
					return err_ok;
				}
				else
				{
					return err_write_concern;// there is a configuration issue with write concern
				}
			}
		}
		else if (item->status == EMPTY || item->status == DELETED)
		{
			//problem is here with base types as it is just an array of data.  Need better way
			item->status = IN_USE;
			memcpy(item->data, key, (hash_map->record.key_size));
			memcpy(item->data + hash_map->record.key_size, value,
			        (hash_map->record.value_size));
#ifdef DEBUG
			{
				int i;
				for (i = 0; i < (hash_map->record.key_size
				                         + hash_map->record.value_size); i++)
				{
					io_printf("%c", *((item->data) + i));
				}
			}
			io_printf(" inserted at location %d\n", loc);
#endif
			count++;

			return err_ok;
		}

		loc++;
		if (loc >= hash_map->map_size)	// Perform wrapping
			loc = 0;

#ifdef DEBUG
		io_printf("checking location %i\n", loc);
#endif
	}

#ifdef DEBUG
	io_printf("Hash table full.  Insert not done");
#endif

	return err_max_capacity;
}

int
oah_findItemLoc(
	hashmap_t 		*hash_map,
	ion_key_t		key
)
{
	hash_t hash 				= hash_map->compute_hash(hash_map, key,
	        						hash_map->record.key_size);
													//compute hash value for given key
	int loc 					= oah_get_location(hash, hash_map->map_size);
													//determine bucket based on hash

	int count 		= 0;
	while (count != hash_map->map_size)
	{
		// check to see if current item is a match based on key
		// locate first item
		hash_bucket_t * item 	= (((hash_bucket_t *)((hash_map->entry
									+ (hash_map->record.key_size
										+ hash_map->record.value_size
											+ SIZEOF(STATUS)) * loc))));

		if (item->status == EMPTY)
		{
			return -1;		//TODO correct
		}
		else //calculate if there is a match
		{
			if (item->status != DELETED)
			{
				//need to check key match; what's the most efficient way?
				int key_is_equal = memcmp(item->data, key,
				        			hash_map->record.key_size);

				if (key_is_equal == IS_EQUAL)
				{
					return loc;
				}
			}
			count++;
			loc++;
			if (loc >= hash_map->map_size)	// Perform wrapping
				loc = 0;
		}
	}
	return -1; //FIXME return codes
}

err_t
oah_delete(
	hashmap_t 		*hash_map,
	ion_key_t 		key
)
{
	int loc	 					= oah_findItemLoc(hash_map, key);

	if (loc == -1)
	{
#ifdef DEBUG
		io_printf("Item not found when trying to oah_delete.\n");
#endif
		return err_item_not_found;
	}
	else
	{
		//locate item
		hash_bucket_t * item 	= (((hash_bucket_t *)((hash_map->entry
		        					+ (hash_map->record.key_size
		        						+ hash_map->record.value_size
		        							+ SIZEOF(STATUS)) * loc))));

		item->status			= DELETED;					//delete item

#ifdef DEBUG
		io_printf("Item deleted at location %d\n", loc);
#endif
		return err_ok;
	}
}

err_t
oah_query(
	hashmap_t 		*hash_map,
	ion_key_t 		key,
	char 			**data)
{
	int loc 					= oah_findItemLoc(hash_map, key);

	io_printf("located item at %i\n", loc);

	if (loc != -1)
	{

#ifdef DEBUG
		io_printf("Item found at location %d\n", loc);
#endif

		int data_length 		= hash_map->record.key_size
		        					+ hash_map->record.value_size;
		hash_bucket_t * item	= (((hash_bucket_t *)((hash_map->entry
		        					+ (data_length + SIZEOF(STATUS)) * loc))));
		*data 					= (char *)malloc(sizeof(char) * (data_length));
		memcpy(*data, (char *)(item->data), data_length);
		return err_ok;
	}
	else
	{
#ifdef DEBUG
		io_printf("Item not found in hash table.\n");
#endif
		return err_item_not_found;
	}
}

void
oah_print(
	hashmap_t 	*hash_map,
	int 		size,
	record_t 	*record
)
{
	int i;
	io_printf("Printing map\n");

	for (i = 0; i < size; i++)
	{
		io_printf("%d -- %i ", i,
			((hash_bucket_t *)((hash_map->entry
				+ (record->key_size + record->value_size
					+ SIZEOF(STATUS)) * i)))->status);
		{
			if (((hash_bucket_t *)((hash_map->entry
				+ (record->key_size + record->value_size + SIZEOF(STATUS))
					* i)))->status == (EMPTY | DELETED))
			{
				io_printf("(null)");
			}
			else
			{
				int j;
				for (j = 0; j < (record->key_size + record->value_size); j++)
				{
					io_printf("%X ",
						*(unsigned char *)(((hash_bucket_t *)((hash_map->entry
							+ (record->key_size + record->value_size
								+ SIZEOF(STATUS)) * i)))->data + j));
				}
			}
			io_printf("\n");
		}
	}
}

hash_t
oah_compute_simple_hash(
	hashmap_t 		*hashmap,
	ion_key_t 		key,
	int 			size_of_key
)
{
	//convert to a hashable value
	hash_t hash 	= ((hash_t)(*(int *)key)) % hashmap->map_size;

	return hash;
}

