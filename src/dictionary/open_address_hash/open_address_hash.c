/******************************************************************************/
/**
@file
@author		Scott Ronald Fazackerley
@brief		Open Address Hash Map
@details	The open address hash map allows non-colliding entries into a hash table

@todo   capture size of map
@todo   prevent duplicate insertions
@todo   When creating the hash-map, need to know something about what is going in it.
		What we need to know if the the size of the key and the size of the data.
		That is all.  Nothing else.
 */
/******************************************************************************/

#include "open_address_hash.h"

err_t
oah_initialize(
	hashmap_t *hashmap,
	hash_t (*hashing_function)(hashmap_t *, ion_key_t, int),
	key_type_t key_type,
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
	hashmap->entry			= (void *) malloc((hashmap->super.record.key_size + hashmap->super.record.value_size + 1) * hashmap->map_size);
	/* Allows for binding of different hash function depending on requirements. */
	hashmap->compute_hash	= (*hashing_function);

	if (NULL == hashmap->entry) {
		return 1;
	}

#if DEBUG
	io_printf("Initializing hash table\n");
#endif

	/* Initialize hash table */
	for (i = 0; i < size; i++) {
		((hash_bucket_t *) (hashmap->entry + ((hashmap->super.record.key_size + hashmap->super.record.value_size + SIZEOF(STATUS)) * i)))->status = EMPTY;
	}

	return 0;
}

int
oah_get_location(
	hash_t	num,
	int		size
) {
	return num % size;
}

err_t
oah_destroy(
	hashmap_t *hash_map
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
		return err_colllection_destruction_error;
	}
}

ion_status_t
oah_update(
	hashmap_t	*hash_map,
	ion_key_t	key,
	ion_value_t value
) {
	/* TODO: lock potentially required */
	write_concern_t current_write_concern = hash_map->write_concern;

	hash_map->write_concern = wc_update;/* change write concern to allow update */

	ion_status_t status = oah_insert(hash_map, key, value);

	hash_map->write_concern = current_write_concern;
	return status;
}

ion_status_t
oah_insert(
	hashmap_t	*hash_map,
	ion_key_t	key,
	ion_value_t value
) {
	hash_t hash = hash_map->compute_hash(hash_map, key, hash_map->super.record.key_size);	/* compute hash value for given key */

	int loc		= oah_get_location(hash, hash_map->map_size);

	/* Scan until find an empty location - oah_insert if found */
	int count	= 0;

	hash_bucket_t *item;

	while (count != hash_map->map_size) {
		item = ((hash_bucket_t *) ((hash_map->entry + (hash_map->super.record.key_size + hash_map->super.record.value_size + SIZEOF(STATUS)) * loc)));

		if (item->status == IN_USE) {
			/* if a cell is in use, need to key to */

			if (hash_map->super.compare(item->data, key, hash_map->super.record.key_size) == IS_EQUAL) {
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
					return ION_STATUS_ERROR(err_write_concern);	/* there is a configuration issue with write concern */
				}
			}
		}
		else if ((item->status == EMPTY) || (item->status == DELETED)) {
			/* problem is here with base types as it is just an array of data.  Need better way */
			item->status = IN_USE;
			memcpy(item->data, key, (hash_map->super.record.key_size));
			memcpy(item->data + hash_map->super.record.key_size, value, (hash_map->super.record.value_size));
			return ION_STATUS_OK(1);
		}

		loc++;

		if (loc >= hash_map->map_size) {
			/* Perform wrapping */
			loc = 0;
		}

#if DEBUG
		io_printf("checking location %i\n", loc);
#endif
		count++;
	}

#if DEBUG
	io_printf("Hash table full.  Insert not done");
#endif

	return ION_STATUS_ERROR(err_max_capacity);
}

err_t
oah_find_item_loc(
	hashmap_t	*hash_map,
	ion_key_t	key,
	int			*location
) {
	/* compute hash value for given key */
	hash_t hash = hash_map->compute_hash(hash_map, key, hash_map->super.record.key_size);

	int loc		= oah_get_location(hash, hash_map->map_size);
	/* determine bucket based on hash */

	int count	= 0;

	while (count != hash_map->map_size) {
		/* check to see if current item is a match based on key */
		/* locate first item */
		hash_bucket_t *item = (((hash_bucket_t *) ((hash_map->entry + (hash_map->super.record.key_size + hash_map->super.record.value_size + SIZEOF(STATUS)) * loc))));

		if (item->status == EMPTY) {
			return err_item_not_found;	/* if you hit an empty cell, exit */
		}
		else {
			/* calculate if there is a match */

			if (item->status != DELETED) {
				/** @todo correct compare to use proper returen type*/
				/** @todo An error exisits with the comparitor from the dictionary and will need to be
				 * revisitied onced fixed */

				int key_is_equal = hash_map->super.compare(item->data, key, hash_map->super.record.key_size);

				if (IS_EQUAL == key_is_equal) {
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
	hashmap_t	*hash_map,
	ion_key_t	key
) {
	int loc = -1;

	if (oah_find_item_loc(hash_map, key, &loc) == err_item_not_found) {
#if DEBUG
		io_printf("Item not found when trying to oah_delete.\n");
#endif
		return ION_STATUS_ERROR(err_item_not_found);
	}
	else {
		/* locate item */
		hash_bucket_t *item = (((hash_bucket_t *) ((hash_map->entry + (hash_map->super.record.key_size + hash_map->super.record.value_size + SIZEOF(STATUS)) * loc))));

		item->status = DELETED;	/* delete item */

#if DEBUG
		io_printf("Item deleted at location %d\n", loc);
#endif
		return ION_STATUS_OK(1);
	}
}

ion_status_t
oah_query(
	hashmap_t	*hash_map,
	ion_key_t	key,
	ion_value_t value
) {
	int loc;

	if (oah_find_item_loc(hash_map, key, &loc) == err_ok) {
#if DEBUG
		io_printf("Item found at location %d\n", loc);
#endif

		int				data_length = hash_map->super.record.key_size + hash_map->super.record.value_size;
		hash_bucket_t	*item		= (((hash_bucket_t *) ((hash_map->entry + (data_length + SIZEOF(STATUS)) * loc))));

		/* *value				   = (ion_value_t)malloc(sizeof(char) * (hash_map->super.record.value_size)); */
		memcpy(value, (ion_value_t) (item->data + hash_map->super.record.key_size), hash_map->super.record.value_size);
		return ION_STATUS_OK(1);
	}
	else {
#if DEBUG
		io_printf("Item not found in hash table.\n");
#endif
		return ION_STATUS_ERROR(err_item_not_found);
	}
}

void
oah_print(
	hashmap_t		*hash_map,
	int				size,
	record_info_t	*record
) {
	int i;

	io_printf("Printing map\n");

	for (i = 0; i < size; i++) {
		io_printf("%d -- %i ", i, ((hash_bucket_t *) ((hash_map->entry + (record->key_size + record->value_size + SIZEOF(STATUS)) * i)))->status);
		{
			if (((hash_bucket_t *) ((hash_map->entry + (record->key_size + record->value_size + SIZEOF(STATUS)) * i)))->status == (EMPTY | DELETED)) {
				io_printf("(null)");
			}
			else {
				int j;

				for (j = 0; j < (record->key_size + record->value_size); j++) {
					io_printf("%X ", *(byte *) (((hash_bucket_t *) ((hash_map->entry + (record->key_size + record->value_size + SIZEOF(STATUS)) * i)))->data + j));
				}
			}

			io_printf("\n");
		}
	}
}

hash_t
oah_compute_simple_hash(
	hashmap_t	*hashmap,
	ion_key_t	key,
	int			size_of_key
) {
	/* convert to a hashable value */
	/** @TODO int will cause an issues depending on sizeof int */
	hash_t hash = ((hash_t) (*(int *) key)) % hashmap->map_size;

	return hash;
}
