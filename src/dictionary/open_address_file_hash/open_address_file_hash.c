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

#include "open_address_file_hash.h"

#define ION_TEST_FILE "file.bin"

ion_err_t
oafh_close(
	ion_file_hashmap_t *hash_map
) {
	if (NULL != hash_map->file) {
		/* check to ensure that you are not freeing something already free */
		fclose(hash_map->file);
		free(hash_map);
		return err_ok;
	}
	else {
		return err_file_close_error;
	}
}

ion_err_t
oafh_initialize(
	ion_file_hashmap_t *hashmap,
	ion_hash_t (*hashing_function)(ion_file_hashmap_t *, ion_key_t, int),
	ion_key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	int size,
	ion_dictionary_id_t id
) {
	hashmap->write_concern				= wc_insert_unique;			/* By default allow unique inserts only */
	hashmap->super.record.key_size		= key_size;
	hashmap->super.record.value_size	= value_size;
	hashmap->super.key_type				= key_type;

	/* The hash map is allocated as a single contiguous file*/
	hashmap->map_size					= size;

	hashmap->compute_hash				= (*hashing_function);	/* Allows for binding of different hash functions
																depending on requirements */

	char addr_filename[ION_MAX_FILENAME_LENGTH];

	/* open the file */
	int actual_filename_length = dictionary_get_filename(id, "oaf", addr_filename);

	if (actual_filename_length >= ION_MAX_FILENAME_LENGTH) {
		return err_dictionary_initialization_failed;
	}

	hashmap->file = fopen(addr_filename, "r+b");

	if (NULL != hashmap->file) {
		return err_ok;
	}

	/* open the file */
	hashmap->file = fopen(addr_filename, "w+b");

	ion_hash_bucket_t *file_record;

	int record_size = ION_SIZEOF(STATUS) + hashmap->super.record.key_size + hashmap->super.record.value_size;

	file_record			= calloc(record_size, 1);
	file_record->status = ION_EMPTY;

	/* write out the records to disk to prep */
#if DEBUG
	printf("Initializing hash table\n");
#endif

	int i, writes = 0;

	for (i = 0; i < hashmap->map_size; i++) {
		writes	+= fwrite(&file_record->status, ION_SIZEOF(STATUS), 1, hashmap->file);
		writes	+= fwrite(file_record->data, record_size - ION_SIZEOF(STATUS), 1, hashmap->file);
	}

	fflush(hashmap->file);

	if (writes / 2 != hashmap->map_size) {
		fclose(hashmap->file);
		return err_file_write_error;
	}

	free(file_record);

	return err_ok;
}

int
oafh_get_location(
	ion_hash_t	num,
	int			size
) {
	return num % size;
}

ion_err_t
oafh_destroy(
	ion_file_hashmap_t *hash_map
) {
	hash_map->compute_hash				= NULL;
	hash_map->map_size					= 0;
	hash_map->super.record.key_size		= 0;
	hash_map->super.record.value_size	= 0;

	char addr_filename[ION_MAX_FILENAME_LENGTH];

	int actual_filename_length = dictionary_get_filename(hash_map->super.id, "oaf", addr_filename);

	if (actual_filename_length >= ION_MAX_FILENAME_LENGTH) {
		return err_dictionary_destruction_error;
	}

	if (hash_map->file != NULL) {
		/* check to ensure that you are not freeing something already free */
		fclose(hash_map->file);
		fremove(addr_filename);
		hash_map->file = NULL;
		return err_ok;
	}
	else {
		return err_dictionary_destruction_error;
	}
}

ion_status_t
oafh_update(
	ion_file_hashmap_t	*hash_map,
	ion_key_t			key,
	ion_value_t			value
) {
	/* TODO: lock potentially required */
	ion_write_concern_t current_write_concern = hash_map->write_concern;

	hash_map->write_concern = wc_update;/* change write concern to allow update */

	ion_status_t result = oafh_insert(hash_map, key, value);

	hash_map->write_concern = current_write_concern;
	return result;
}

ion_status_t
oafh_insert(
	ion_file_hashmap_t	*hash_map,
	ion_key_t			key,
	ion_value_t			value
) {
	ion_hash_t hash = hash_map->compute_hash(hash_map, key, hash_map->super.record.key_size);	/* compute hash value for given key */

	int loc			= oafh_get_location(hash, hash_map->map_size);

	/* Scan until find an empty location - oah_insert if found */
	int count		= 0;

	ion_hash_bucket_t *item;

	int record_size = hash_map->super.record.key_size + hash_map->super.record.value_size + ION_SIZEOF(STATUS);

	item = malloc(record_size);

	/* set file position */
	fseek(hash_map->file, loc * record_size, SEEK_SET);

	while (count != hash_map->map_size) {
		fread(item, record_size, 1, hash_map->file);
#if DEBUG
		DUMP((int) ftell(hash_map->file), "%i");
#endif

		if (item->status == ION_IN_USE) {
			/* if a cell is in use, need to key to */

			if (hash_map->super.compare(item->data, key, hash_map->super.record.key_size) == IS_EQUAL) {
				if (hash_map->write_concern == wc_insert_unique) {
					/* allow unique entries only */
					free(item);
					return ION_STATUS_ERROR(err_duplicate_key);
				}
				else if (hash_map->write_concern == wc_update) {
					/* allows for values to be updated											// */
					/* backup and write */
					fseek(hash_map->file, ION_SIZEOF(STATUS) + hash_map->super.record.key_size - record_size, SEEK_CUR);
#if DEBUG
					DUMP((int) ftell(hash_map->file), "%i");
					DUMP(value, "%s");
#endif
					fwrite(value, hash_map->super.record.value_size, 1, hash_map->file);
					free(item);
					return ION_STATUS_OK(1);
				}
				else {
					free(item);
					return ION_STATUS_ERROR(err_write_concern);	/* there is a configuration issue with write concern */
				}
			}
		}
		else if ((item->status == ION_EMPTY) || (item->status == ION_DELETED)) {
			/* problem is here with base types as it is just an array of data.  Need better way */
			/* printf("empty\n"); */
			fseek(hash_map->file, -record_size, SEEK_CUR);
#if DEBUG
			DUMP((int) ftell(hash_map->file), "%i");
#endif
			item->status = ION_IN_USE;
			memcpy(item->data, key, (hash_map->super.record.key_size));
			memcpy(item->data + hash_map->super.record.key_size, value, (hash_map->super.record.value_size));
			fwrite(item, record_size, 1, hash_map->file);
			free(item);

			return ION_STATUS_OK(1);
		}

		loc++;

		if (loc >= hash_map->map_size) {
			/* Perform wrapping */
			loc = 0;
			/* rewind the file */
			frewind(hash_map->file);
		}

#if DEBUG
		printf("checking location %i\n", loc);
#endif
		count++;
	}

#if DEBUG
	printf("Hash table full.  Insert not done");
#endif
	free(item);
	return ION_STATUS_ERROR(err_max_capacity);
}

ion_err_t
oafh_find_item_loc(
	ion_file_hashmap_t	*hash_map,
	ion_key_t			key,
	int					*location
) {
	ion_hash_t hash = hash_map->compute_hash(hash_map, key, hash_map->super.record.key_size);
	/* compute hash value for given key */

	int loc			= oafh_get_location(hash, hash_map->map_size);
	/* determine bucket based on hash */

	int count		= 0;

	ion_hash_bucket_t *item;

	int record_size = hash_map->super.record.key_size + hash_map->super.record.value_size + ION_SIZEOF(STATUS);

	item = malloc(record_size);

	/* set file position */
	fseek(hash_map->file, loc * record_size, SEEK_SET);

	/* needs to traverse file again */
	while (count != hash_map->map_size) {
		fread(&item->status, ION_SIZEOF(STATUS), 1, hash_map->file);
		fread(item->data, record_size - ION_SIZEOF(STATUS), 1, hash_map->file);

		if (item->status == ION_EMPTY) {
			free(item);
			return err_item_not_found;	/* if you hit an empty cell, exit */
		}
		else {
			/* calculate if there is a match */

			if (item->status != ION_DELETED) {
				/*@todo correct compare to use proper return type*/
				int key_is_equal = hash_map->super.compare(item->data, key, hash_map->super.record.key_size);

				if (IS_EQUAL == key_is_equal) {
					(*location) = loc;
					free(item);
					return err_ok;
				}
			}

			loc++;
			count++;

			if (loc >= hash_map->map_size) {
				/* Perform wrapping */
				loc = 0;
				frewind(hash_map->file);
			}
		}
	}

	free(item);
	return err_item_not_found;	/* key have not been found */
}

ion_status_t
oafh_delete(
	ion_file_hashmap_t	*hash_map,
	ion_key_t			key
) {
	int loc;

	if (oafh_find_item_loc(hash_map, key, &loc) == err_item_not_found) {
#if DEBUG
		printf("Item not found when trying to oah_delete.\n");
#endif
		return ION_STATUS_ERROR(err_item_not_found);
	}
	else {
		/* locate item */
		ion_hash_bucket_t *item;

		int record_size = hash_map->super.record.key_size + hash_map->super.record.value_size + ION_SIZEOF(STATUS);

		item = malloc(record_size);

		/* set file position */
		fseek(hash_map->file, loc * record_size, SEEK_SET);

		fread(&item->status, ION_SIZEOF(STATUS), 1, hash_map->file);
		fread(item->data, record_size - ION_SIZEOF(STATUS), 1, hash_map->file);

		item->status = ION_DELETED;	/* delete item */

		/* backup */
		fseek(hash_map->file, -record_size, SEEK_CUR);
		fwrite(&item->status, ION_SIZEOF(STATUS), 1, hash_map->file);
		fwrite(item->data, record_size - ION_SIZEOF(STATUS), 1, hash_map->file);

		free(item);
#if DEBUG
		printf("Item deleted at location %d\n", loc);
#endif
		return ION_STATUS_OK(1);
	}
}

ion_status_t
oafh_query(
	ion_file_hashmap_t	*hash_map,
	ion_key_t			key,
	ion_value_t			value
) {
	int loc;

	if (oafh_find_item_loc(hash_map, key, &loc) == err_ok) {
#if DEBUG
		printf("Item found at location %d\n", loc);
#endif

		int record_size = hash_map->super.record.key_size + hash_map->super.record.value_size + ION_SIZEOF(STATUS);

		/* set file position */
		fseek(hash_map->file, (loc * record_size) + ION_SIZEOF(STATUS) + hash_map->super.record.key_size, SEEK_SET);
#if DEBUG
		printf("seeking %i\n", (loc * record_size) + SIZEOF(STATUS) + hash_map->super.record.key_size);
#endif
		fread(value, hash_map->super.record.value_size, 1, hash_map->file);

		return ION_STATUS_OK(1);
	}
	else {
#if DEBUG
		printf("Item not found in hash table.\n");
#endif
		value = NULL;	/*et the number of bytes to 0 */
		return ION_STATUS_ERROR(err_item_not_found);
	}
}

ion_hash_t
oafh_compute_simple_hash(
	ion_file_hashmap_t	*hashmap,
	ion_key_t			key,
	int					size_of_key
) {
	UNUSED(size_of_key);

	/* convert to a hashable value */
	ion_hash_t hash = (ion_hash_t) (((*(int *) key) % hashmap->map_size) + hashmap->map_size) % hashmap->map_size;

	return hash;
}
