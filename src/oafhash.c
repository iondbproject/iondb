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

#include "oafhash.h"

#define TEST_FILE	"file.bin"

err_t
oafh_initialize(
    file_hashmap_t 		*hashmap,
    hash_t 				(*hashing_function)(file_hashmap_t *, ion_key_t, int),
    key_type_t			key_type,
    ion_key_size_t 		key_size,
    ion_value_size_t 	value_size,
    int 				size
)
{
	hashmap->write_concern 				= wc_insert_unique;			/* By default allow unique inserts only */
	hashmap->super.record.key_size 		= key_size;
	hashmap->super.record.value_size 	= value_size;
	hashmap->super.key_type 			= key_type;

/*	hashmap->compare = compare;*/

	/* The hash map is allocated as a single contiguous file*/
	hashmap->map_size 			= size;

	//open the file
	hashmap->file = fopen(TEST_FILE,"w+");

 /*= (void *)malloc(
	        (hashmap->super.record.key_size + hashmap->super.record.value_size + 1)
	                * hashmap->map_size);*/

	hash_bucket_t *file_record;

	int record_size = SIZEOF(STATUS) + hashmap->super.record.key_size + hashmap->super.record.value_size;

	file_record = (hash_bucket_t *)malloc(record_size);
	file_record->status = EMPTY;

	//write out the records to disk to prep
#if DEBUG
	io_printf("Initializing hash table\n");
#endif

	int i, writes = 0;
	for (i = 0; i<hashmap->map_size ; i++)
	{
		writes += fwrite(file_record,record_size,1,hashmap->file);
	}
	fflush(hashmap->file);
	if (writes != hashmap->map_size)
	{
		fclose(hashmap->file);
		return err_file_write_error;
	}

	hashmap->compute_hash 		= (*hashing_function);		/* Allows for binding of different hash functions
																depending on requirements */

#if DEBUG
	io_printf("Record key size: %i\n", hashmap->super.record.key_size);
	io_printf("Record value size: %i\n", hashmap->super.record.value_size);
	io_printf("Size of map (in bytes): %i\n",
	        (hashmap->record.key_size + hashmap->super.record.value_size + 1)
	                * hashmap->map_size);
#endif

	return err_ok;
}

int
oafh_get_location(
	hash_t 		num,
	int 		size
)
{
	return num % size;
}

err_t
oafh_destroy(
	file_hashmap_t 	*hash_map
)
{
	hash_map->compute_hash 		= NULL;
	hash_map->map_size 			= 0;
	hash_map->super.record.key_size 	= 0;
	hash_map->super.record.value_size	= 0;
	if (hash_map->file != NULL)			//check to ensure that you are not freeing something already free
	{
		fclose(hash_map->file);
#if ARDUINO == 1
		fremove(TEST_FILE);
#else
		remove(TEST_FILE);
#endif
		hash_map->file = NULL;				//
		return err_ok;
	}
	else
	{
		return err_colllection_destruction_error;
	}
}

err_t
oafh_update(
	file_hashmap_t	 	*hash_map,
	ion_key_t 		key,
	ion_value_t		value
)
{
	//TODO: lock potentially required
	write_concern_t current_write_concern 	= hash_map->write_concern;
	hash_map->write_concern 				= wc_update;			//change write concern to allow update
	err_t result 							= oafh_insert(hash_map, key, value);
	hash_map->write_concern 				= current_write_concern;
	return result;
}

err_t
oafh_insert(
	file_hashmap_t 		*hash_map,
	ion_key_t 		key,
	ion_value_t 	value
)
{


#if DEBUG
	int i;

	for (i = 0; i < hash_map->record.key_size; i++)
	{
		io_printf("%x ", key[i]);
	}
	io_printf("\n");
#endif

	hash_t hash 	= hash_map->compute_hash(hash_map, key,
							hash_map->super.record.key_size);			//compute hash value for given key

#if DEBUG
	io_printf("hash value is %i\n", hash);
	io_printf("data value is ");
	for (i = 0; i < hash_map->record.value_size; i++)
	{
		io_printf("%c", *(value + i));
	}
	io_printf("\n");
#endif

	int loc			= oafh_get_location(hash, hash_map->map_size);

#if DEBUG
	DUMP(loc,"%i");
#endif

	// Scan until find an empty location - oah_insert if found
	int count 		= 0;

	hash_bucket_t *item;

	int record_size = hash_map->super.record.key_size + hash_map->super.record.value_size
			+ SIZEOF(STATUS);

	item = (hash_bucket_t *)malloc(record_size);

	//set file position
	fseek(hash_map->file, loc * record_size, SEEK_SET);

	while (count != hash_map->map_size)
	{
		/*item 		= ((hash_bucket_t *)((hash_map->entry
		        			+ (hash_map->super.record.key_size + hash_map->super.record.value_size
		        				+ SIZEOF(STATUS)) * loc)));*/

		fread(item,record_size,1,hash_map->file);
#if DEBUG
		DUMP((int)ftell(hash_map->file),"%i");
#endif
		if (item->status == IN_USE) 	//if a cell is in use, need to key to
		{
			/*if (memcmp(item->data, key, hash_map->record.key_size) == IS_EQUAL)*/
			if (hash_map->super.compare(item->data, key, hash_map->super.record.key_size) == IS_EQUAL)
			{
				if (hash_map->write_concern == wc_insert_unique)//allow unique entries only
				{
					free(item);
					return err_duplicate_key;
				}
				else if (hash_map->write_concern == wc_update)//allows for values to be updated											//
				{
					//backup and write
					fseek(hash_map->file, SIZEOF(STATUS) + hash_map->super.record.key_size -record_size, SEEK_CUR);
#if DEBUG
					DUMP((int)ftell(hash_map->file),"%i");
					DUMP(value,"%s");
#endif
					fwrite(value, hash_map->super.record.value_size, 1, hash_map->file);
					free(item);
					return err_ok;
				}
				else
				{
					free(item);
					return err_write_concern;// there is a configuration issue with write concern
				}
			}

		}
		else if (item->status == EMPTY || item->status == DELETED)
		{
			//problem is here with base types as it is just an array of data.  Need better way
			//printf("empty\n");
			fseek(hash_map->file,-record_size,SEEK_CUR);
#if DEBUG
			DUMP((int)ftell(hash_map->file),"%i");
#endif
			item->status = IN_USE;
			memcpy(item->data, key, (hash_map->super.record.key_size));
			memcpy(item->data + hash_map->super.record.key_size, value,
			        (hash_map->super.record.value_size));
			fwrite(item, record_size, 1, hash_map->file);
			free(item);

#if DEBUG
			{
				int i;
				for (i = 0; i < (hash_map->record.key_size
				                         + hash_map->super->record.value_size); i++)
				{
					io_printf("%c", *((item->data) + i));
				}
			}
			io_printf(" inserted at location %d\n", loc);
#endif
			return err_ok;
		}

		loc++;
		if (loc >= hash_map->map_size)	// Perform wrapping
		{
			loc = 0;
			//rewind the file
			rewind(hash_map->file);
		}

#if DEBUG
		io_printf("checking location %i\n", loc);
#endif
		count++;
	}

#if DEBUG
	io_printf("Hash table full.  Insert not done");
#endif
	free(item);
	return err_max_capacity;
}

err_t
oafh_find_item_loc(
	file_hashmap_t 	*hash_map,
	ion_key_t		key,
	int				*location
)
{

	hash_t hash 				= hash_map->compute_hash(hash_map, key,
	        						hash_map->super.record.key_size);
													//compute hash value for given key

	int loc 					= oafh_get_location(hash, hash_map->map_size);
													//determine bucket based on hash

	int count 					= 0;

	hash_bucket_t *item;

	int record_size = hash_map->super.record.key_size + hash_map->super.record.value_size
			+ SIZEOF(STATUS);

	item = (hash_bucket_t *)malloc(record_size);

	//set file position
	fseek(hash_map->file, loc * record_size, SEEK_SET);

	//needs to traverse file again
	while (count != hash_map->map_size)
	{

		fread(item,record_size,1,hash_map->file);

		if (item->status == EMPTY)
		{
			free(item);
			return err_item_not_found;		//if you hit an empty cell, exit
		}
		else //calculate if there is a match
		{
			if (item->status != DELETED)
			{
				/** @todo correct compare to use proper return type*/
				int key_is_equal 	= hash_map->super.compare(item->data, key, hash_map->super.record.key_size);

				if (IS_EQUAL == key_is_equal)
				{
					(*location) = loc;
					free(item);
					return err_ok;
				}
			}
			loc++;
			count++;
			if (loc >= hash_map->map_size)	// Perform wrapping
			{
				loc = 0;
				rewind(hash_map->file);
			}
		}

	}
	free(item);
	return err_item_not_found; 				//key have not been found
}

err_t
oafh_delete(
	file_hashmap_t 		*hash_map,
	ion_key_t 		key
)
{
	int loc;

	if (oafh_find_item_loc(hash_map, key, &loc) == err_item_not_found)
	{
#if DEBUG
		io_printf("Item not found when trying to oah_delete.\n");
#endif
		return err_item_not_found;
	}
	else
	{
		//locate item
		hash_bucket_t *item;

		int record_size = hash_map->super.record.key_size + hash_map->super.record.value_size
				+ SIZEOF(STATUS);

		item = (hash_bucket_t *)malloc(record_size);

			//set file position
		fseek(hash_map->file, loc * record_size, SEEK_SET);

		fread(item,record_size,1,hash_map->file);

/*		hash_bucket_t * item 	= (((hash_bucket_t *)((hash_map->entry
		        					+ (hash_map->super.record.key_size
		        						+ hash_map->super.record.value_size
		        							+ SIZEOF(STATUS)) * loc))));*/

		item->status			= DELETED;					//delete item

		//backup
		fseek(hash_map->file, -record_size, SEEK_CUR);
		fwrite(item, record_size, 1, hash_map->file);
		free(item);
#if DEBUG
		io_printf("Item deleted at location %d\n", loc);
#endif
		return err_ok;
	}
}

err_t
oafh_query(
	file_hashmap_t 		*hash_map,
	ion_key_t 		key,
	ion_value_t		value)
{
	int loc;

	if (oafh_find_item_loc(hash_map, key, &loc) == err_ok)
	{
#if DEBUG
		io_printf("Item found at location %d\n", loc);
#endif

		int record_size = hash_map->super.record.key_size
				+ hash_map->super.record.value_size
				+ SIZEOF(STATUS);

			//set file position
		fseek(hash_map->file, (loc * record_size) + SIZEOF(STATUS) + hash_map->super.record.key_size, SEEK_SET);
#if DEBUG
		printf("seeking %i\n",(loc * record_size) + SIZEOF(STATUS) + hash_map->super.record.key_size);
#endif
		fread(value, hash_map->super.record.value_size, 1, hash_map->file);

		/**value 					= (ion_value_t)malloc(sizeof(char) * (hash_map->super.record.value_size));
		memcpy(*value, (ion_value_t)(item->data+hash_map->super.record.key_size), hash_map->super.record.value_size);*/
		return err_ok;
	}
	else
	{
#if DEBUG
		io_printf("Item not found in hash table.\n");
#endif
		value = NULL;				/**set the number of bytes to 0 */
		return err_item_not_found;
	}
}



hash_t
oafh_compute_simple_hash(
	file_hashmap_t 		*hashmap,
	ion_key_t 		key,
	int 			size_of_key
)
{
	//convert to a hashable value
	hash_t hash 	= ((hash_t)(*(int *)key)) % hashmap->map_size;

	return hash;
}



