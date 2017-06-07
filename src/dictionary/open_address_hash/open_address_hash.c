/******************************************************************************/
/**
@file
@author		Scott Ronald Fazackerley
@brief		Open Address Hash Map
@details	The open address hash map allows non-colliding entries into a hash table
*/
/******************************************************************************/

#include "open_address_hash.h"

ion_err_t
oah_initialize(
	ion_hashmap_t *hashmap,
	ion_hash_t (*hashing_function)(ion_hashmap_t *, ion_key_t, int),
	ion_key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	int size
) {
	int i;

	hashmap->write_concern				= wc_insert_unique;			/* By default allow unique inserts only */
	hashmap->super.record.key_size		= key_size;
	hashmap->super.record.value_size	= value_size;
	hashmap->super.key_type				= key_type;

/*	hashmap->compare = compare;*/

	/* The hash map is allocated as a single contiguous array*/
	hashmap->map_size		= size;
	hashmap->entry			= malloc((hashmap->super.record.key_size + hashmap->super.record.value_size + 1) * hashmap->map_size);
	/* Allows for binding of different hash function depending on requirements. */
	hashmap->compute_hash	= (*hashing_function);

	if (NULL == hashmap->entry) {
		return 1;
	}

#if ION_DEBUG
	printf("Initializing hash table\n");
#endif

	/* Initialize hash table */
	for (i = 0; i < size; i++) {
		((ion_hash_bucket_t *) (hashmap->entry + ((hashmap->super.record.key_size + hashmap->super.record.value_size + SIZEOF(STATUS)) * i)))->status = ION_EMPTY;
	}

	return 0;
}

int
oah_get_location(
	ion_hash_t	num,
	int			size
) {
	return num % size;
}

ion_err_t
oah_destroy(
	ion_hashmap_t *hash_map
) {
	hash_map->compute_hash				= NULL;
	hash_map->map_size					= 0;
	hash_map->super.record.key_size		= 0;
	hash_map->super.record.value_size	= 0;

	if (hash_map->entry != NULL) {
		/* check to ensure that you are not freeing something already free */
		free(hash_map->entry);
		hash_map->entry = NULL;	/*  */
		return err_ok;
	}
	else {
		return err_dictionary_destruction_error;
	}
}

ion_status_t
oah_update(
	ion_hashmap_t	*hash_map,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_write_concern_t current_write_concern = hash_map->write_concern;

	hash_map->write_concern = wc_update;/* change write concern to allow update */

	ion_status_t status = oah_insert(hash_map, key, value);

	hash_map->write_concern = current_write_concern;
	return status;
}

ion_status_t
oah_insert(
	ion_hashmap_t	*hash_map,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_hash_t hash = hash_map->compute_hash(hash_map, key, hash_map->super.record.key_size);	/* compute hash value for given key */

	int loc			= oah_get_location(hash, hash_map->map_size);

	/* Scan until find an empty location - oah_insert if found */
	int count		= 0;

	ion_hash_bucket_t *item;

	while (count != hash_map->map_size) {
		item = ((ion_hash_bucket_t *) ((hash_map->entry + (hash_map->super.record.key_size + hash_map->super.record.value_size + SIZEOF(STATUS)) * loc)));

		if (item->status == ION_IN_USE) {
			/* if a cell is in use, need to key to */

			if (hash_map->super.compare(item->data, key, hash_map->super.record.key_size) == ION_IS_EQUAL) {
				if (hash_map->write_concern == wc_insert_unique) {
					/* allow unique entries only */
					return ION_STATUS_ERROR(err_duplicate_key);
				}
				else if (hash_map->write_concern == wc_update) {
					/* allows for values to be updated */
					memcpy(item->data + hash_map->super.record.key_size, value, (hash_map->super.record.value_size));
					return ION_STATUS_OK(1);
				}
				else {
					return ION_STATUS_ERROR(err_file_write_error);	/* there is a configuration issue with write concern */
				}
			}
		}
		else if ((item->status == ION_EMPTY) || (item->status == ION_DELETED)) {
			/* problem is here with base types as it is just an array of data.  Need better way */
			item->status = ION_IN_USE;
			memcpy(item->data, key, (hash_map->super.record.key_size));
			memcpy(item->data + hash_map->super.record.key_size, value, (hash_map->super.record.value_size));
			return ION_STATUS_OK(1);
		}

		loc++;

		if (loc >= hash_map->map_size) {
			/* Perform wrapping */
			loc = 0;
		}

#if ION_DEBUG
		printf("checking location %i\n", loc);
#endif
		count++;
	}

#if ION_DEBUG
	printf("Hash table full.  Insert not done");
#endif

	return ION_STATUS_ERROR(err_max_capacity);
}

ion_err_t
oah_find_item_loc(
	ion_hashmap_t	*hash_map,
	ion_key_t		key,
	int				*location
) {
	/* compute hash value for given key */
	ion_hash_t hash = hash_map->compute_hash(hash_map, key, hash_map->super.record.key_size);

	int loc			= oah_get_location(hash, hash_map->map_size);
	/* determine bucket based on hash */

	int count		= 0;

	while (count != hash_map->map_size) {
		/* check to see if current item is a match based on key */
		/* locate first item */
		ion_hash_bucket_t *item = (((ion_hash_bucket_t *) ((hash_map->entry + (hash_map->super.record.key_size + hash_map->super.record.value_size + SIZEOF(STATUS)) * loc))));

		if (item->status == ION_EMPTY) {
			return err_item_not_found;	/* if you hit an empty cell, exit */
		}
		else {
			/* calculate if there is a match */

			if (item->status != ION_DELETED) {
				int key_is_equal = hash_map->super.compare(item->data, key, hash_map->super.record.key_size);

				if (ION_IS_EQUAL == key_is_equal) {
					(*location) = loc;
					return err_ok;
				}
			}

			count++;
			loc++;

			if (loc >= hash_map->map_size) {
				/* Perform wrapping */
				loc = 0;
			}
		}
	}

	return err_item_not_found;	/* key have not been found */
}

ion_status_t
oah_delete(
	ion_hashmap_t	*hash_map,
	ion_key_t		key
) {
	int loc = -1;

	if (oah_find_item_loc(hash_map, key, &loc) == err_item_not_found) {
#if ION_DEBUG
		printf("Item not found when trying to oah_delete.\n");
#endif
		return ION_STATUS_ERROR(err_item_not_found);
	}
	else {
		/* locate item */
		ion_hash_bucket_t *item = (((ion_hash_bucket_t *) ((hash_map->entry + (hash_map->super.record.key_size + hash_map->super.record.value_size + SIZEOF(STATUS)) * loc))));

		item->status = ION_DELETED;	/* delete item */

#if ION_DEBUG
		printf("Item deleted at location %d\n", loc);
#endif
		return ION_STATUS_OK(1);
	}
}

ion_status_t
oah_query(
	ion_hashmap_t	*hash_map,
	ion_key_t		key,
	ion_value_t		value
) {
	int loc;

	if (oah_find_item_loc(hash_map, key, &loc) == err_ok) {
#if ION_DEBUG
		printf("Item found at location %d\n", loc);
#endif

		int					data_length = hash_map->super.record.key_size + hash_map->super.record.value_size;
		ion_hash_bucket_t	*item		= (((ion_hash_bucket_t *) ((hash_map->entry + (data_length + SIZEOF(STATUS)) * loc))));

		/* *value				   = malloc(sizeof(char) * (hash_map->super.record.value_size)); */
		memcpy(value, (item->data + hash_map->super.record.key_size), hash_map->super.record.value_size);
		return ION_STATUS_OK(1);
	}
	else {
#if ION_DEBUG
		printf("Item not found in hash table.\n");
#endif
		return ION_STATUS_ERROR(err_item_not_found);
	}
}

/**
@brief		Helper function to print out map.

@details	Helper function that displays the contents of the map including
			both key and value.

@param		hash_map
				The map into which the data is going to be inserted.
@param		size
				The number of buckets available in the map.
@param		record
				The structure of the record being inserted.
*/
void
oah_print(
	ion_hashmap_t		*hash_map,
	int					size,
	ion_record_info_t	*record
) {
	int i;

	printf("Printing map\n");

	for (i = 0; i < size; i++) {
		printf("%d -- %i ", i, ((ion_hash_bucket_t *) ((hash_map->entry + (record->key_size + record->value_size + SIZEOF(STATUS)) * i)))->status);
		{
			if (((ion_hash_bucket_t *) ((hash_map->entry + (record->key_size + record->value_size + SIZEOF(STATUS)) * i)))->status == (ION_EMPTY | ION_DELETED)) {
				printf("(null)");
			}
			else {
				int j;

				for (j = 0; j < (record->key_size + record->value_size); j++) {
					printf("%X ", *(ion_byte_t *) (((ion_hash_bucket_t *) ((hash_map->entry + (record->key_size + record->value_size + SIZEOF(STATUS)) * i)))->data + j));
				}
			}

			printf("\n");
		}
	}
}

ion_hash_t
oah_compute_simple_hash(
	ion_hashmap_t	*hashmap,
	ion_key_t		key,
	int				size_of_key
) {
	UNUSED(size_of_key);

	ion_hash_t hash = (ion_hash_t) (((*(int *) key) % hashmap->map_size) + hashmap->map_size) % hashmap->map_size;

	return hash;
}
