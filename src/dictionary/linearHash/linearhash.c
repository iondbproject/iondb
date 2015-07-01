/******************************************************************************/
/**
@file		linearhash.c
@author		Scott Ronald Fazackerley
@brief		Linear Hash
@details	The linear hash map allows non-colliding entries into a disk based hash table

@todo 	capture size of map
@todo 	prevent duplicate insertions
@todo  	When creating the hash-map, need to know something about what is going in it.
 		What we need to know if the the size of the key and the size of the data.
 		That is all.  Nothing else.
 */
/******************************************************************************/

#include "linearhash.h"

#define TEST_FILE	"lh_main.bin"
#define OVERFLOW_FILE "lh_overflow.bin"
#define INVALID -1


err_t
lh_initialize(
    linear_hashmap_t 	*hashmap,
    err_t 				(*compute_hash)(linear_hashmap_t *, ion_key_t, int, int, hash_set_t *),
    key_type_t			key_type,
    ion_key_size_t 		key_size,
    ion_value_size_t 	value_size,
    int 				size,
    int					id
)
{
	hashmap->write_concern 				= wc_insert_unique;			/* By default allow unique inserts only */
	hashmap->file_level					= 0;
	hashmap->bucket_pointer 			= 0;
	hashmap->super.record.key_size 		= key_size;
	hashmap->super.record.value_size 	= value_size;
	hashmap->super.key_type 			= key_type;
	hashmap->id							= id;

	if ( size < 2 || ( 1 << (int)floor(log2(size))) != size)
	{
		return err_invalid_initial_size;
	}


	hashmap->initial_map_size 			= size;	 		/* @todo this needs to be 2^n*/
#if DEBUG
	DUMP(hashmap->initial_map_size,"%i");
#endif

	char filename[12];
	sprintf(filename,"%i_%s",id,TEST_FILE);
	//open the file
	hashmap->file = fopen(filename,"w+b");		//main hash file

	//initital the hash map with a min number of buckets
	//Assumes that there is only one record per bucket
	/** @todo Correct the minimum block size as this will need to be increased to improve performance */
	l_hash_bucket_t *file_record;
	int record_size = SIZEOF(STATUS) + hashmap->super.record.key_size + hashmap->super.record.value_size;
	file_record = (l_hash_bucket_t *)malloc(record_size);
	file_record->status = EMPTY;

	//write out the records to disk to prep
#if DEBUG
	io_printf("Initializing hash table\n");
#endif

	int writes = 0;
	int write_count = 0;

#if DEBUG
	DUMP(record_size,"%i");
	DUMP(hashmap->initial_map_size,"%i");
	DUMP(hashmap->file,"%p");
#endif

	/** prep all the pages per bucket */
	for (writes = 0; writes < hashmap->initial_map_size*RECORDS_PER_BUCKET; writes++)
	{
		write_count += fwrite(file_record,record_size,1,hashmap->file);
	}
	fflush(hashmap->file);

#if DEBUG
	DUMP(write_count,"%i");
#endif

	//check to make sure file has been written correctly
	if (write_count != hashmap->initial_map_size * RECORDS_PER_BUCKET)
	{
		fclose(hashmap->file);
		free(file_record);
		return err_file_write_error;
	}

	hashmap->compute_hash 		= compute_hash;		/* Allows for binding of different hash functions
																depending on requirements */

#if DEBUG
	io_printf("Record key size: %i\n", hashmap->super.record.key_size);
	io_printf("Record value size: %i\n", hashmap->super.record.value_size);
	io_printf("Size of map (in bytes): %i\n",
	        (hashmap->record.key_size + hashmap->super.record.value_size + 1)
	                * hashmap->map_size);
#endif


	free(file_record);

	/** remove overflow files */
	return err_ok;
}

err_t
lh_close(
	linear_hashmap_t	*hash_map
	)
{
	if (fclose(hash_map->file) == 0)
	{
		return err_ok;
	}
	else
	{
		return err_file_close_error;
	}
}


err_t
lh_destroy(
	linear_hashmap_t 	*hash_map
)
{
	/** close all the overflow files associated with LH*/
	int bucket_idx;
	char  	filename[20];
	err_t error = err_ok;

	for (bucket_idx = 0; bucket_idx < (hash_map->initial_map_size*(1 << hash_map->file_level)+hash_map->bucket_pointer); bucket_idx ++)
	{
		char 	*extension = "ovf";
		sprintf(filename,".\\%i_%i.%s",hash_map->id,bucket_idx,extension);
		//allocation space for file name
		FILE * bucket_file;
		if ((bucket_file = fopen(filename,"rb")) != NULL)
		{
			if (fclose(bucket_file) == 0)
			{
				free(bucket_file);
#if ARDUINO == 1

				if ( fremove(filename) != 0)
				{
					error = err_colllection_destruction_error;
				}
#else
				if ( remove(filename) != 0)
				{
					error = err_colllection_destruction_error;
				}
#endif
			}
			else
			{
				error = err_colllection_destruction_error;
			}
		}
	}
	if (hash_map->file != NULL)			//check to ensure that you are not freeing something already free
	{
		fclose(hash_map->file);
		sprintf(filename,"%i_%s",hash_map->id,TEST_FILE);			/** @todo fix name */
#if ARDUINO == 1

		if ( fremove(filename) != 0)
		{
			error = err_colllection_destruction_error;
		}
#else
		if ( remove(filename) != 0)
		{
			error = err_colllection_destruction_error;
		}
#endif
		hash_map->file = NULL;				//
		hash_map->compute_hash 				= NULL;
		hash_map->initial_map_size			= 0;
		hash_map->super.record.key_size 	= 0;
		hash_map->super.record.value_size	= 0;
		return error;
	}
	else
	{
		return err_colllection_destruction_error;
	}
}

err_t
lh_update(
	linear_hashmap_t	 	*hash_map,
	ion_key_t 				key,
	ion_value_t				value
)
{
	//TODO: lock potentially required
	write_concern_t current_write_concern 	= hash_map->write_concern;
	hash_map->write_concern 				= wc_update;			//change write concern to allow update
	err_t result 							= lh_insert(hash_map, key, value);
	hash_map->write_concern 				= current_write_concern;
	return result;
}

err_t
lh_insert(
	linear_hashmap_t	*hash_map,
	ion_key_t 			key,
	ion_value_t 		value
)
{
/** @todo this needs to be update! */

	l_hash_bucket_t * bucket;


#if DEBUG
	int i;

	for (i = 0; i < hash_map->record.key_size; i++)
	{
		io_printf("%x ", key[i]);
	}
	io_printf("\n");
#endif

	hash_set_t hash_set;						/** used to store hash values when determining location */
	/* Find the inital location for the entry */
	hash_map->compute_hash(hash_map, key,hash_map->super.record.key_size,hash_map->file_level, &hash_set);

#if DEBUG
	io_printf("lower hash value is %i\n", hash_set.lower_hash);
	io_printf("upper hash value is %i\n", hash_set.upper_hash);
#endif

	int record_size = hash_map->super.record.key_size + hash_map->super.record.value_size
					+ SIZEOF(STATUS);

	int bucket_size = record_size * RECORDS_PER_BUCKET;
	int bucket_number;

#if DEBUG
	DUMP(bucket_size,"%i");
#endif

	bucket = (l_hash_bucket_t *)malloc(bucket_size);			/** allocate memory for page */

	if (hash_set.lower_hash >= hash_map->bucket_pointer)						/** if the lower hash is below pointer, then use upper hash */
	{
#if DEBUG
		DUMP(hash_set.lower_hash * bucket_size,"%i");
#endif
		fseek(hash_map->file, hash_set.lower_hash * bucket_size, SEEK_SET);
		bucket_number = hash_set.lower_hash;
	}
	else
	{
		fseek(hash_map->file, hash_set.upper_hash * bucket_size, SEEK_SET);
		bucket_number = hash_set.upper_hash;
	}

	//read is the bucket and scan for an empty page
	fread(bucket,bucket_size,1,hash_map->file);

	// Scan until find an empty location
	int count 		= 0;

	l_hash_bucket_t *item;

	//check to see if value is in primary bucket
	while (count != RECORDS_PER_BUCKET)
	{
		item = (l_hash_bucket_t * )(bucket + ( count * record_size));				/** advance through page */
#if DEBUG
		DUMP(count * record_size,"%i");
#endif
		/** scan through the entire block looking for a space */
		/** @todo need to address duplicate keys */

		if (item->status != IN_USE )	/** if location is not being used, use it */
		{
			item->status = IN_USE;
			memcpy(item->data, key, (hash_map->super.record.key_size));
			memcpy(item->data + hash_map->super.record.key_size, value,
						        (hash_map->super.record.value_size));
#if DEBUG
			DUMP(*(int*)item->data,"%i");
#endif
			/** and write block back out */
			fseek(hash_map->file, -(bucket_size),SEEK_CUR);
			fwrite(bucket,bucket_size, 1, hash_map->file);
			free(bucket);
			return err_ok;
		}
		count++;
	}
	/** add to overflow page if need be if there is no room*/
#if DEBUG
	io_printf("Overflow!\n");
#endif
	ll_file_t linked_list_file;
	if (fll_open(
			&linked_list_file,
			fll_compare,
			hash_map->super.key_type,
			hash_map->super.record.key_size,
		    hash_map->super.record.value_size,
		    bucket_number,
		    hash_map->id)
			== err_item_not_found)
	{
		fll_create(
				&linked_list_file,
				fll_compare,
				hash_map->super.key_type,
				hash_map->super.record.key_size,
				hash_map->super.record.value_size,
				bucket_number,
				hash_map->id
				);
	}
	ll_file_node_t * node;
	fll_create_node(&linked_list_file,&(hash_map->super.record),key,value,&node);
	fll_insert(&linked_list_file,node);
	fll_close(&linked_list_file);
	free(node);

	return err_ok;
}

err_t
lh_split(
	linear_hashmap_t	*hash_map
)
{
	//splits the current bucket into two new buckets

	hash_set_t hash_set;						/** used to store hash values when determining location */


	int record_size = hash_map->super.record.key_size + hash_map->super.record.value_size
						+ SIZEOF(STATUS);

	int bucket_size = record_size * RECORDS_PER_BUCKET;

	l_hash_bucket_t * bucket = (l_hash_bucket_t *)malloc(bucket_size);			/** allocate memory for page from disk */

	l_hash_bucket_t * upper_bucket = (l_hash_bucket_t *)malloc(bucket_size);	/** allocate memory for page */

	fseek(hash_map->file, hash_map->bucket_pointer * bucket_size, SEEK_SET);	/** select bucket to split */

	fread(bucket,bucket_size,1,hash_map->file);

	int count = 0;
	int lower_bucket_idx = 0, upper_bucket_idx = 0;
	l_hash_bucket_t * item;

	while (count != RECORDS_PER_BUCKET)
	{
		item = (l_hash_bucket_t * )(bucket + ( count * record_size));			/** advance through page */

		//scan through the entire block looking for a space

		if (item->status == IN_USE )	/** if location is not being used, use it */
		{
			//determine new bucket
			hash_map->compute_hash(hash_map,(ion_key_t)item->data,hash_map->super.record.key_size,hash_map->file_level, &hash_set);

			if (hash_set.lower_hash != hash_set.upper_hash)				/** move this record to the new bucket */
			{
				memcpy(upper_bucket+upper_bucket_idx*record_size, item,record_size);
				upper_bucket_idx++;										/** advance counter to next record */
				item->status = DELETED;									/** and delete record from bucket */
			}
		}
		count++;
	}

	/** a new block has been created, but you now need to denote the rest of the positions as available */
	{
		int tmp_idx = upper_bucket_idx;
		while (tmp_idx++ < RECORDS_PER_BUCKET)
		{
			item = (l_hash_bucket_t * )(bucket + ( count * record_size));
			item->status = EMPTY;
			memcpy(upper_bucket+upper_bucket_idx*record_size, item,record_size);
		}
	}

	/** now that the bucket has been split, the overflow file needs to also be spit
	 *  and it is known that depending on the split there will be at least room in
	 *  one of the two buckets */
	//write
#if DEBUG
	io_printf("splitting bucket %i\n",hash_map->bucket_pointer);
#endif
	/** open the file to split */
	ll_file_t split_ll;

	if (fll_open(
			&split_ll,
			NULL,
			hash_map->super.key_type,
			hash_map->super.record.key_size,
			hash_map->super.record.value_size,
			hash_map->bucket_pointer,
			hash_map->id
			) != err_item_not_found)
	{
		/** split file */
		ll_file_t new_ll;														/** new ll for bucket */
		fll_create(&new_ll,
			NULL,
			hash_map->super.key_type,
			hash_map->super.record.key_size,
			hash_map->super.record.value_size,
			hash_set.upper_hash,
			hash_map->id);

		ll_file_node_t * ll_node = (ll_file_node_t*)malloc(split_ll.node_size);

		fll_reset(&split_ll);

		while (fll_next(&split_ll,ll_node) != err_item_not_found)
		{
#if DEBUG
			DUMP(*(int*)ll_node->data,"%i");
#endif
			hash_map->compute_hash(hash_map,(ion_key_t)ll_node->data,hash_map->super.record.key_size,hash_map->file_level, &hash_set);
#if DEBUG
			DUMP(hash_set.lower_hash,"%i");
			DUMP(hash_set.upper_hash,"%i");
#endif
			/** @todo - this needs an explanation */
			if (hash_set.lower_hash == hash_set.upper_hash)			/** then this stays in the same location */
			{
				if (lower_bucket_idx <RECORDS_PER_BUCKET)			//still room in buffer
				{//check to see if there is room in lower bucket
					while (lower_bucket_idx < RECORDS_PER_BUCKET)
						{
							item = (l_hash_bucket_t * )(bucket + ( lower_bucket_idx * record_size));			/** advance through page */
							//scan through the entire block looking for a space

							if (item->status != IN_USE )	/** if location is not being used, use it */
							{
								item->status = IN_USE;
								memcpy(item->data,ll_node->data,record_size);		/** copy in record */
								fll_remove(&split_ll);								/** and remove record */
							}
							lower_bucket_idx++;										/** advance counter to next record */
						}
				}//and of not, just leave it in the overflow file
			}
			else
			{
				//check to see if there is room in the upper bucket
				if (upper_bucket_idx < RECORDS_PER_BUCKET)			//still room in buffer
				{	/** check to see if there is room in upper bucket.
				 	 	 as this is new, just keep adding to it */
					item = (l_hash_bucket_t * )(upper_bucket+upper_bucket_idx*record_size);
																					/** advance through page */
					item->status = IN_USE;
					memcpy(item->data, ll_node->data,record_size);
#if DEBUG
					DUMP(*(int *)item->data,"%i");
#endif
					upper_bucket_idx++;												/** advance counter to next record */
				}else				//and if not, add to file
				{
					fll_insert(&new_ll,ll_node);									/** just copy over the node */
				}
				/** and remove node from list */
				fll_remove(&split_ll);
			}
		}
		free(ll_node);
		fll_close(&split_ll);
		fll_close(&new_ll);
	}
	/** and finally flush out the pages */
	fseek(hash_map->file, hash_map->bucket_pointer * bucket_size, SEEK_SET);	/**write bucket back to disk*/
	fwrite(bucket,bucket_size,1,hash_map->file);
	fseek(hash_map->file, 0, SEEK_END);											/** and add new bucket */
	fwrite(upper_bucket,bucket_size,1,hash_map->file);
	/** increment page pointers, et al. */
	hash_map->bucket_pointer++;
	if (hash_map->bucket_pointer == hash_map->initial_map_size*(1<<hash_map->file_level))
	{
		//if you have consumed all buckets at this level, reset and move on*/
		hash_map->bucket_pointer = 0;
		hash_map->file_level ++;
	}
	free(bucket);
	free(upper_bucket);
	return err_ok;
}

err_t
lh_delete(
	linear_hashmap_t 	*hash_map,
	ion_key_t 			key
)
{

	hash_set_t hash_set;

	err_t err = hash_map->compute_hash(hash_map,key,hash_map->super.record.key_size,hash_map->file_level,&hash_set);
#if DEBUG
	DUMP(*(int*)key,"%i");
	DUMP(hash_set.lower_hash,"%i");
	DUMP(hash_set.upper_hash,"%i");
#endif
	if (err == err_uninitialized)
	{
		return err_uninitialized;
	}

	int record_size = hash_map->super.record.key_size + hash_map->super.record.value_size
						+ SIZEOF(STATUS);
	int bucket_size = record_size * RECORDS_PER_BUCKET;
	int num_deleted = 0;
	int bucket_number;

#if DEBUG
	DUMP(bucket_size,"%i");
#endif

	l_hash_bucket_t * bucket = (l_hash_bucket_t *)malloc(bucket_size);			/** allocate memory for page */

	if (hash_set.lower_hash >= hash_map->bucket_pointer)						/** if the lower hash is below pointer, then use upper hash */
	{
#if DEBUG
		DUMP(hash_set.lower_hash * bucket_size,"%i");
#endif
		fseek(hash_map->file, hash_set.lower_hash * bucket_size, SEEK_SET);
		bucket_number = hash_set.lower_hash;
	}
	else
	{
		fseek(hash_map->file, hash_set.upper_hash * bucket_size, SEEK_SET);
		bucket_number = hash_set.upper_hash;
	}

	//read is the bucket and scan for an empty page
	fread(bucket,bucket_size,1,hash_map->file);

	// Scan until find an empty location
	int count 		= 0;

	l_hash_bucket_t *item;

	//check to see if value is in primary bucket
	while (count != RECORDS_PER_BUCKET)
	{
		item = (l_hash_bucket_t * )(bucket + ( count * record_size));				/** advance through page */
#if DEBUG
		DUMP(count * record_size,"%i");
#endif
		//scan through the entire block looking for a space

		if (item->status == IN_USE )	/** if location is not being used, use it */
		{
			/**if it's in use, compare the keys */
			if (hash_map->super.compare(key,item->data,hash_map->super.record.key_size) == IS_EQUAL)
			{
				item->status = DELETED;												/** Change status */
				num_deleted++;
			}
		}
		count++;
	}
	fseek(hash_map->file,-bucket_size,SEEK_CUR);									/** backup and write back */
	fwrite(bucket,bucket_size,1,hash_map->file);
	free(bucket);

	/** and if it is not found in the bucket, check the overflow file */
#if DEBUG
	io_printf("checking overflow page\n");
#endif
	ll_file_t linked_list_file;
	if (fll_open(
			&linked_list_file,
			fll_compare,
			hash_map->super.key_type,
			hash_map->super.record.key_size,
			hash_map->super.record.value_size,
			bucket_number,
			hash_map->id)
			== err_item_not_found)
	{

	}
	else
	{
		ion_value_t value = (ion_value_t)malloc(hash_map->super.record.value_size);
		fll_reset(&linked_list_file);
		/*while(fll_next(&linked_list_file,&ll_node) != err_item_not_found)	* consume each node in the file
		{
			int isequal = hash_map->super.compare(ll_node.data,key,hash_map->super.record.key_size);
			if (isequal >= 1)			* then you have already passed all possible value, so exit
			{
				break;
			}
			else if (isequal == 0)											* then a match has been found
			{
				fll_remove(&linked_list_file);
				num_deleted++;
			}
		}*/
		printf("starting deletes\n");
		while (err_item_not_found != lh_get_next(hash_map, &linked_list_file,key,value))
		{
			fll_remove(&linked_list_file);
			num_deleted++;
		}
		free(value);
	}

	fll_close(&linked_list_file);											/** and close the file */
	if (num_deleted != 0)
	{
		return err_ok;
	}
	else
	{
		return err_item_not_found;
	}
}

err_t
lh_find(
		dictionary_t 	*dictionary,
		predicate_t 	*predicate,
		dict_cursor_t 	**cursor
)
{
	//find a value that satisfies the cre

	return err_ok;
}

int lh_compute_bucket_number(
	  linear_hashmap_t			*hash_map,
	  hash_set_t				*hash_set
  )
{
	int bucket_number = -1;

	if (hash_set->lower_hash >= hash_map->bucket_pointer) /** if the lower hash is below pointer, then use upper hash */
	{
#if DEBUG
		DUMP(hash_set.lower_hash * bucket_size,"%i");
#endif
		bucket_number = hash_set->lower_hash;
	}
	else
	{
		bucket_number = hash_set->upper_hash;
	}
	return bucket_number;
}

/**
 * @brief finds the value in the primary page.
 * @param hash_map
 * @param bucket_number
 * @param key
 * @param value
 * @return
 */
err_t
lh_search_primary_page(
    linear_hashmap_t		*hash_map,
    int						bucket_number,
    ion_key_t				key,
    ion_value_t				value
  )
{
	int record_size = hash_map->super.record.key_size
	        + hash_map->super.record.value_size + SIZEOF(STATUS);
	int bucket_size = record_size * RECORDS_PER_BUCKET;
	l_hash_bucket_t* bucket = (l_hash_bucket_t*)malloc(bucket_size); /** allocate memory for page */

	fseek(hash_map->file, bucket_number * bucket_size, SEEK_SET);
	//read is the bucket and scan for an empty page
	fread(bucket, bucket_size, 1, hash_map->file);
	// Scan until find an empty location
	int count = 0;
	l_hash_bucket_t* item;
	//check to see if value is in primary bucket
	while (count != RECORDS_PER_BUCKET)
	{
		item = (l_hash_bucket_t *)(bucket + (count * record_size)); /** advance through page */
#if DEBUG
		DUMP(count * record_size,"%i");
#endif
		/** scan through the entire block looking for a space */
		/** @todo need to address duplicate keys */
		if (item->status == EMPTY) /** nothing is here, so exit */
		{
			value = NULL;
			free(bucket);
			return err_item_not_found;	/** in this case, the cursor would be null */
		}
		else if (item->status == IN_USE) /** if location is not being used, use it */
		{
			/**if it's in use, compare the keys */
			if (hash_map->super.compare(key, item->data,
			        hash_map->super.record.key_size) == IS_EQUAL)
			{
				memcpy(value, item->data + hash_map->super.record.key_size,
				        hash_map->super.record.value_size);
				/** in this case the cursor will be set to be count in bucket
				 * and the value cached? */
				free(bucket);
				return err_ok;
			}
		}
		count++;
	}
	free(bucket);
	return err_not_in_primary_page;
}

err_t
lh_get_next(
    linear_hashmap_t			*hash_map,
    ll_file_t					*linked_list_file,
    ion_key_t 					key,
    ion_value_t 				value
)
{
	ll_file_node_t * ll_node = (ll_file_node_t *)malloc(linked_list_file->node_size);

	while (fll_next(linked_list_file, ll_node) != err_item_not_found)/** consume each node in the file */
	{
		int isequal = hash_map->super.compare(ll_node->data, key,
		        hash_map->super.record.key_size);
		/** use satisfy predicate -> need to build quick cursor to evaluate */
		//int isequal = lhdict_test_predicate();
		if (isequal >= 1)/** then you have already passed all possible value, so exit */
		{
			//value = NULL;
			//fll_close(linked_list_file);
			free(ll_node);
			return err_item_not_found;
		}
		else if (isequal == 0)/** then a match has been found */
		{
			memcpy(value, ll_node->data + hash_map->super.record.key_size,
			        hash_map->super.record.value_size);
			//fll_close(&*linked_list_file);
			free(ll_node);
			return err_ok;
		}
	}
	return err_item_not_found;
}

err_t
lh_query(
	linear_hashmap_t 	*hash_map,
	ion_key_t 			key,
	ion_value_t			value)
{

	/** compute possible hash set for key */
	hash_set_t hash_set;
	err_t err = hash_map->compute_hash(hash_map,key,hash_map->super.record.key_size,hash_map->file_level,&hash_set);
#if DEBUG
	DUMP(*(int*)key,"%i");
	DUMP(hash_set.lower_hash,"%i");
	DUMP(hash_set.upper_hash,"%i");
#endif

	if (err == err_uninitialized)
	{
		return err_uninitialized;
	}

	/** compute the primary page to search */
	int bucket_number = lh_compute_bucket_number(hash_map, &hash_set);

	/** and search it */
	err = lh_search_primary_page(hash_map, bucket_number, key, value);
	/** and if the primary page has empty slot or has the value, return */
	if (err != err_not_in_primary_page)
	{
		return err;
	}

#if DEBUG
	DUMP(bucket_size,"%i");
#endif

	/** and if it is not found in the bucket, check the overflow file */
	/** In practice, this could be open ?? */

	ll_file_t linked_list_file;

	if (fll_open(&linked_list_file,
			fll_compare,
			hash_map->super.key_type,
			hash_map->super.record.key_size,
			hash_map->super.record.value_size,
			bucket_number,
			hash_map->id)
			== err_item_not_found)
	{
		/** in this case, cursor is null as value has not been found */
		return  err_item_not_found;
	}
	else
	{
		// find the next available node that matches ?
		/** @TODO change possibly to satisfies predicate?*/
		fll_reset(&linked_list_file);
		/** this scans the list for the first instance of the item in the ll */
		err = lh_get_next(hash_map, &linked_list_file, key, value);
		fll_close(&linked_list_file);										/** close the list and be done as you have reached the end without finding value */
		return err;
	}
}




err_t
lh_compute_hash(
		linear_hashmap_t 	*hashmap,
		ion_key_t 			key,
		int 				size_of_key,
		int					file_level,
		hash_set_t			*hash_set
)
{
		//convert to a hashable value
		if (hash_set == NULL)
		{
			return err_uninitialized;
		}
		else
		{
			/** @todo Endian issue */
			hash_set->lower_hash = ((hash_t)(*(int *)key)) % ((1 << file_level) * hashmap->initial_map_size);
			hash_set->upper_hash = ((hash_t)(*(int *)key)) % ((1 << (file_level+1)) * hashmap->initial_map_size);
			return err_ok;
		}
}




