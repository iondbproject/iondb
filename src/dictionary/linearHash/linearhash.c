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
	hashmap->id							= id;						/** @ todo this can be removed as id is now part of the parent */
	hashmap->record_size				= key_size + value_size + SIZEOF(STATUS);
																	/** record size, while it can be computed on the fly is a commonly computed value
																	 * 	and adds significant overhead so it can be precomputed */

	int i;
	for (i = 0; i < CACHE_SIZE; i++)
	{
		hashmap->cache[i].status			= cache_invalid;
	}
	if ( size < 2 || ( 1 << (int)floor(log2(size))) != size)
	{
		return err_invalid_initial_size;
	}


	hashmap->initial_map_size 			= size;	 		/* @todo this needs to be 2^n*/
#if DEBUG
	DUMP(hashmap->initial_map_size,"%i");
#endif

	char filename[14];
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
/*	io_printf("Size of map (in bytes): %i\n",
	        (hashmap->record.key_size + hashmap->super.record.value_size + 1)
	                * hashmap->map_size);*/
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
	int cache_idx;
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
				//free(bucket_file);
				//bucket_file = NULL;
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
		hash_map->file						= NULL;
		hash_map->compute_hash 				= NULL;
		hash_map->initial_map_size			= 0;
		hash_map->super.record.key_size 	= 0;
		hash_map->super.record.value_size	= 0;

		/** @TODO should cache blocks be flushed ?*/
		for (cache_idx = 0; cache_idx < CACHE_SIZE; cache_idx++)
		{
			if (hash_map->cache[cache_idx].status != cache_invalid)		/** free any cache blocks that are left */
			{
				free(hash_map->cache[cache_idx].cached_bucket);
			}
		}
		return error;
	}
	else
	{
		return err_colllection_destruction_error;
	}
}

/** @TODO */
err_t
lh_update(
	linear_hashmap_t	 	*hash_map,
	ion_key_t 				key,
	ion_value_t				value
)
{
	/** updates all values for a given key and if not found, then it is inserted */
	write_concern_t current_write_concern 	= hash_map->write_concern;
	hash_map->write_concern 				= wc_update;								//change write concern to allow update
	err_t result 							= lh_insert(hash_map, key, value);			//updates all values for a given key
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

	lh_page_cache_t * cache = NULL;

	/** Bring the primary page into cache */
	lh_cache_pp(hash_map,0,bucket_number,&cache);

	/** @FIXME - need cache block mgr */
	return_status_t status = lh_action_primary_page(hash_map, cache, bucket_number, key, lh_insert_item_action,value);

	if (status.err == err_ok)
	{
#if DEBUG
		io_printf("flushing!\n");
#endif
		lh_flush_cache(hash_map,cache,PRESERVE_CACHE_MEMORY);
		return status.err;
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
	ll_file_node_t * node;					/** Must be destroyed when done */
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
	/** splits the current bucket into two new buckets */
	hash_set_t hash_set;										/** used to store hash values when determining location */
	int lower_cache_number = 0;
	int upper_cache_number = 1;

	/** cache the current page to split */
	lh_page_cache_t	* lower_cache;
	lh_cache_pp(hash_map,lower_cache_number,hash_map->bucket_pointer,&lower_cache);

	/** upper is new page */
	lh_page_cache_t	* upper_cache;
	lh_cache_pp(hash_map,upper_cache_number,EMPTY_BLOCK_REQUEST,&upper_cache);

#if DEBUG
	DUMP(lower_cache->status,"%i");
	DUMP(lower_cache->cached_bucket,"%p");
	DUMP(upper_cache->status,"%i");
	DUMP(upper_cache->cached_bucket,"%p");
#endif

	int count = 0;
	int lower_bucket_idx = 0, upper_bucket_idx = 0;
	l_hash_bucket_t * item;

	while (count != RECORDS_PER_BUCKET)
	{
																							/** @FIXME */
		/** advance through page */
		lh_read_cache(hash_map,lower_cache,count,(void*)&item);

		/** scan through the entire block looking for a space */
		if (item->status == IN_USE )	/** if location is not being used, use it */
		{
			//determine new bucket
			hash_map->compute_hash(hash_map,(ion_key_t)item->data,hash_map->super.record.key_size,hash_map->file_level, &hash_set);

			if (hash_set.lower_hash != hash_set.upper_hash)				//* move this record to the new bucket
			{
																							/** @FIXME */
				lh_write_cache_raw(hash_map, upper_cache, (upper_cache->cached_bucket + upper_bucket_idx*hash_map->record_size), item);
				upper_bucket_idx++;																//* advance counter to next record
				lh_write_record_status_in_cache(hash_map,item,DELETED);
			}
		}
		count++;
	}

	/** a new block has been created, but you now need to denote the rest of the positions as available */
	{
		int tmp_idx = upper_bucket_idx;
		while (tmp_idx < RECORDS_PER_BUCKET)
		{
			/** @FIXME */
			lh_read_cache(hash_map,upper_cache,tmp_idx,(void*)&item);
			lh_write_record_status_in_cache(hash_map,item,EMPTY);
			tmp_idx ++;
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
			&split_ll, NULL, hash_map->super.key_type, hash_map->super.record.key_size, hash_map->super.record.value_size, hash_map->bucket_pointer, hash_map->id
			) != err_item_not_found)
	{
		/** split file */
		ll_file_t new_ll;														/** new ll for bucket */

		fll_create(&new_ll, NULL, hash_map->super.key_type, hash_map->super.record.key_size, hash_map->super.record.value_size, hash_set.upper_hash, hash_map->id);

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
				{	/**check to see if there is room in lower bucket */
					while (lower_bucket_idx < RECORDS_PER_BUCKET)
						{
							lh_read_cache(hash_map,lower_cache,lower_bucket_idx,(void*)&item);
							/** advance through page looking for a space */
							if (item->status != IN_USE )	/** if location is not being used, use it */
							{
								//item->status = IN_USE;		/** @FIXME consider using function to change status but as item is being written out */
								lh_write_record_status_in_cache(hash_map,item,IN_USE);
								lh_write_cache_raw(hash_map,lower_cache,item->data,(void *)ll_node->data);
																					/** copy in record which should change the state of the cache */
								fll_remove(&split_ll);								/** and remove record */
							}
							lower_bucket_idx++;										/** advance counter to next record */
						}
				}	/**and of not, just leave it in the overflow file */
			}
			else
			{
				//check to see if there is room in the upper bucket
				if (upper_bucket_idx < RECORDS_PER_BUCKET)			//still room in buffer
				{	/** check to see if there is room in upper bucket.
				 	 	 as this is new, just keep adding to it */
					lh_read_cache(hash_map,upper_cache,upper_bucket_idx,(void*)&item);
																					/** advance through page */
					//item->status = IN_USE;										/** this may be faster but violates cache integrity */
					lh_write_record_status_in_cache(hash_map,item,IN_USE);
					lh_write_cache_raw(hash_map,upper_cache,item->data,(void*)ll_node->data);
																					/** copy in record which should change the state of the cache */
#if DEBUG
					DUMP(*(int *)item->data,"%i");
#endif
					upper_bucket_idx++;												/** advance counter to next record */
				}else																/** and if not, add to file */
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
	lh_flush_cache(hash_map,lower_cache,PRESERVE_CACHE_MEMORY);
	lh_flush_cache(hash_map,upper_cache,PRESERVE_CACHE_MEMORY);
	/** increment page pointers, et al. */
	hash_map->bucket_pointer++;
	if (hash_map->bucket_pointer == hash_map->initial_map_size*(1<<hash_map->file_level))
	{
		//if you have consumed all buckets at this level, reset and move on*/
		hash_map->bucket_pointer = 0;
		hash_map->file_level ++;
	}
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
		return err;
	}

	/** compute the primary page to search */
	int bucket_number = lh_compute_bucket_number(hash_map, &hash_set);

	int num_deleted = 0;

	/**cache primary page*/
	lh_page_cache_t	* cache;

	lh_cache_pp(hash_map,0,bucket_number,&cache);

#if DEBUG
	//DUMP(bucket_size,"%i");
#endif

	/** function pointer for action function when bringing in block*/
	return_status_t status = lh_action_primary_page(hash_map, cache,  bucket_number, key, lh_delete_item_action, NULL);

	num_deleted = status.count;

	lh_flush_cache(hash_map,cache,PRESERVE_CACHE_MEMORY);

	/** and if it is not found in the bucket, check the overflow file */
#if DEBUG
	io_printf("checking overflow page\n");
#endif
	ll_file_t linked_list_file;
	if (fll_open(
			&linked_list_file, fll_compare, hash_map->super.key_type, hash_map->super.record.key_size, hash_map->super.record.value_size, bucket_number, hash_map->id
			) != err_item_not_found)
	{
		ion_value_t value = (ion_value_t)malloc(hash_map->super.record.value_size);
		fll_reset(&linked_list_file);
		while (err_item_not_found != lh_get_next(hash_map, &linked_list_file,key,value))
		{
			fll_remove(&linked_list_file);
			num_deleted++;
		}
		free(value);

		fll_close(&linked_list_file);											/** and close the file */
	}
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
		linear_hashmap_t	*hash_map,
		dict_cursor_t 		*cursor
)
{
	lh_page_cache_t	* cache;

	//find a value that satisfies the predicate
	switch ((cursor)->predicate->type)
	{
		/**find the page where this first item is*/
		case predicate_equality:
		{//compute hash for key
				/** compute possible hash set for key for the equality value*/
				hash_set_t hash_set;
				err_t err = hash_map->compute_hash(hash_map,cursor->predicate->statement.equality.equality_value,hash_map->super.record.key_size,hash_map->file_level,&hash_set);
	#if DEBUGkey
				DUMP(*(int*)key,"%i");
				DUMP(hash_set.lower_hash,"%i");
				DUMP(hash_set.upper_hash,"%i");
	#endif
				if (err == err_uninitialized)
				{
					return err_uninitialized;
				}

				/** compute the primary page to search */
				((lhdict_cursor_t *)cursor)->first_bucket = lh_compute_bucket_number(hash_map, &hash_set);
				((lhdict_cursor_t *)cursor)->current_bucket = ((lhdict_cursor_t *)cursor)->first_bucket;
				/** and search it as the value must be in the page or the overflow page  */

				lh_cache_pp(hash_map,0,((lhdict_cursor_t *)cursor)->first_bucket,&cache); 		/** cache and leave page */
#if DEBUG
				DUMP(((lhdict_cursor_t *)cursor)->first_bucket,"%i");
#endif

				if (err_ok 	== lh_search_primary_page(hash_map, cache, (lhdict_cursor_t *)cursor)) 		/** then the value has been found and the cursor is good to consume */
				{
					cursor->status = cs_cursor_initialized;
					return err_ok;
				}
				else		/** Search the overflow page  */
				{
					/** @TODO error handling for memory */

					((lhdict_cursor_t *)cursor)->overflow = (ll_file_t *)malloc(sizeof(ll_file_t));

					if (fll_open(
							((lhdict_cursor_t *)cursor)->overflow, fll_compare, hash_map->super.key_type, hash_map->super.record.key_size, hash_map->super.record.value_size, ((lhdict_cursor_t *)cursor)->first_bucket, hash_map->id
							) == err_item_not_found)
					{
						/** in this case, cursor is null as value has not been found */
						cursor->status = cs_end_of_results;
						free(((lhdict_cursor_t *)cursor)->overflow);
						return  err_item_not_found;
					}
					else
					{
						// find the next available node that matches ?
						/** @TODO change possibly to satisfies predicate?*/
						fll_reset(((lhdict_cursor_t *)cursor)->overflow);
						/** this scans the list for the first instance of the item in the ll */
						ll_file_node_t * ll_node = (ll_file_node_t *)malloc(((lhdict_cursor_t *)cursor)->overflow->node_size);
						while (fll_next(((lhdict_cursor_t *)cursor)->overflow, ll_node) != err_item_not_found)
																			/** consume each node in the file */
						{
							troolean_t value = ((lhdict_cursor_t *)cursor)->evaluate_predicate((dict_cursor_t*)cursor,ll_node->data);
							if (IS_GREATER == value)						/** If the value is not found on a strict equality, exit as it will be no where else */
							{
								break;										/** exit and clean up as there are no results */
							}
							else if (IS_EQUAL == value)
							{
								/** and if it satisfies the predicate, the value has been found
								 * @TODO this value could be cached for better performance */
								cursor->status = cs_cursor_initialized;
								free(ll_node);
								return err_ok;
							}
						}
						cursor->status = cs_end_of_results;
						free(ll_node);
						return err_item_not_found;
					}
				}
			break;
		case predicate_range:
		{
			/** compute possible hash set for key for the lower bounded value*/
			hash_set_t hash_set;
			err_t err = hash_map->compute_hash(hash_map,cursor->predicate->statement.range.geq_value,hash_map->super.record.key_size,hash_map->file_level,&hash_set);
	#if DEBUGkey
			DUMP(*(int*)key,"%i");
			DUMP(hash_set.lower_hash,"%i");
			DUMP(hash_set.upper_hash,"%i");
	#endif
			if (err == err_uninitialized)
			{
				return err_uninitialized;
			}

			/** compute the primary page to search*/
			((lhdict_cursor_t *)cursor)->first_bucket = lh_compute_bucket_number(hash_map, &hash_set);
			((lhdict_cursor_t *)cursor)->current_bucket = ((lhdict_cursor_t *)cursor)->first_bucket;

			/** Compute the current size of the linear hash*/
			int current_size = hash_map->initial_map_size * (1 << hash_map->file_level) + hash_map->bucket_pointer;

			do
			{
				/**Multiple pages must be searched in the event a value can not be found */
				lh_cache_pp(hash_map,0,((lhdict_cursor_t *)cursor)->current_bucket,&cache); 		/** cache next PP */
#if DEBUG
				DUMP(((lhdict_cursor_t *)cursor)->current_bucket,"%i");
#endif
				if (err_ok 	== lh_search_primary_page(hash_map, cache, (lhdict_cursor_t *)cursor))
				/** then the value has been found and the cursor is good to consume*/
				{
					cursor->status = cs_cursor_initialized;
					return err_ok;
				}
				else			/** and search it as the value must be in the page or the overflow page*/
				{
					/** @TODO error handling for memory*/
					((lhdict_cursor_t *)cursor)->overflow = (ll_file_t *)malloc(sizeof(ll_file_t));

					if (fll_open(((lhdict_cursor_t *)cursor)->overflow,
							fll_compare,
							hash_map->super.key_type,
							hash_map->super.record.key_size,
							hash_map->super.record.value_size,
							((lhdict_cursor_t *)cursor)->current_bucket,
							hash_map->id)
							== err_item_not_found)
					{

							free(((lhdict_cursor_t *)cursor)->overflow);
							((lhdict_cursor_t *)cursor)->overflow = NULL;
							/** Free up the ll file pointer and move onto the next pp as the next bucket could
							 * contain a value that satisfies the predicate under evaluation */
					}
					else
					{
						/**find the next available node that matches */
						/** @TODO change possibly to satisfies predicate?*/
						fll_reset(((lhdict_cursor_t *)cursor)->overflow);
						/** this scans the list for the first instance of the item in the ll*/
						ll_file_node_t * ll_node = (ll_file_node_t *)malloc(((lhdict_cursor_t *)cursor)->overflow->node_size);
						while (fll_next(((lhdict_cursor_t *)cursor)->overflow, ll_node) != err_item_not_found) /** consume each node in the file*/
						{
							troolean_t value = ((lhdict_cursor_t *)cursor)->evaluate_predicate((dict_cursor_t*)cursor,ll_node->data);
#if DEBUG
							io_printf("Evaluating key %i\n",*(int*)ll_node->data);
#endif
							if (IS_GREATER == value)		/** If the value is not found in the predicate range,
															 *	exit as it will need to check the next pp*/
							{
									/** If the value is about the upper range, then leave ll and move onto the next bucket*/
								break;													/** exit while and continue*/
							}
							else if (IS_EQUAL == value)
							{
								/** and if it satisfies the predicate, the value has been found
								 * @TODO this value could be cached for better performance */
								cursor->status = cs_cursor_initialized;
								free(ll_node);
								return err_ok;											/** Exit with cursor initialized */
							}
						}
						fll_close(((lhdict_cursor_t *)cursor)->overflow);	/** Take care of memory mgmt */
						free(((lhdict_cursor_t *)cursor)->overflow);		/** Remove overflow file pntr */
						((lhdict_cursor_t *)cursor)->overflow = NULL;		/** Clear pointer reference !!Important!! Used by
						 	 	 	 	 	 	 	 	 	 	 	 	 	 *	system to determine if a overflow page is active */
						free(ll_node);
					}
					/** This is no longer the case as system must consider if all buckets have been checked */
					/** increment the search to the next bucket and wrap if necessary*/
					((lhdict_cursor_t *)cursor)->current_bucket = (((lhdict_cursor_t *)cursor)->current_bucket + 1) % current_size;
					/** Reset record pointer as system is now scanning new pp */
					((lhdict_cursor_t *)cursor)->record_pntr = 0;
				}

			}
			while (((lhdict_cursor_t *)cursor)->current_bucket != ((lhdict_cursor_t *)cursor)->first_bucket);
			/** The scan has checked all buckets and has not found a value that satisfies predicate */
			cursor->status = cs_end_of_results;
			return err_item_not_found;
		}
			break;
		default:
			break;
		}
	}
	return err_ok;	/** @TODO what happens here when it reaches the end? */
}

int lh_compute_bucket_number(
	  linear_hashmap_t			*hash_map,
	  hash_set_t				*hash_set
  )
{
	int bucket_number = -1;

	if (hash_set->lower_hash >= hash_map->bucket_pointer) /** if the lower hash is below pointer, then use upper hash */
	{
		bucket_number = hash_set->lower_hash;
	}
	else
	{
		bucket_number = hash_set->upper_hash;
	}
	return bucket_number;
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

	while (fll_next(linked_list_file, ll_node) != err_item_not_found)
																/** consume each node in the file */
	{
		int isequal = hash_map->super.compare(ll_node->data, key,
		        hash_map->super.record.key_size);
		/** use satisfy predicate -> need to build quick cursor to evaluate */
		if (isequal >= 1)										/** then you have already passed all possible value, so exit */
		{
			free(ll_node);
			return err_item_not_found;
		}
		else if (isequal == 0)									/** then a match has been found */
		{
			memcpy(value, ll_node->data + hash_map->super.record.key_size,
			        hash_map->super.record.value_size);
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
	lh_page_cache_t	* cache;

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
	/** @FIXME need cache block mgr */

	lh_cache_pp(hash_map,0,bucket_number,&cache);

	return_status_t status = lh_action_primary_page(hash_map, cache, bucket_number, key, lh_query_item_action, value);

	/** and if the primary page has empty slot or has the value, return */
	if (status.err != err_not_in_primary_page)
	{
		return status.err;
	}

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

action_status_t
lh_query_item_action(
	linear_hashmap_t	*hash_map,
	ion_key_t			key,
	l_hash_bucket_t		*item,
	ion_value_t			value
)
{
	action_status_t status;

	if (item->status == EMPTY) /** nothing is here, so exit */
	{
		value = NULL;
			/** in this case, the cursor would be null */
		status.err = err_item_not_found;
		status.action = action_exit;
		return status;
	}
	else if (item->status == IN_USE) 		/** if location is not being used, use it */
	{
		/**if it's in use, compare the keys */
		if (hash_map->super.compare(key, item->data,
				hash_map->super.record.key_size) == IS_EQUAL)
		{
			memcpy(value, item->data + hash_map->super.record.key_size,
					hash_map->super.record.value_size);
			status.err = err_ok;
			status.action = action_exit;
			return status;
		}
	}
	status.err = err_not_in_primary_page;		/** Assume default that it cannot be found */
	status.action = action_continue;
	return status;
}

action_status_t
lh_insert_item_action(
	linear_hashmap_t	*hash_map,
	ion_key_t			key,
	l_hash_bucket_t		*item,
	ion_value_t			value
	)
{
	action_status_t status;
	if (item->status != IN_USE )	/** if location is not being used, use it */
	{
		lh_write_cache_record(hash_map,item,key,value);
#if DEBUG
		DUMP(*(int*)item->data,"%i");
#endif
		status.err = err_ok;
		status.action = action_exit;
		return status;
	}
	status.err = err_unable_to_insert;
	status.action = action_continue;
	return status;
}

action_status_t
lh_delete_item_action(
	linear_hashmap_t	*hash_map,
	ion_key_t			key,
	l_hash_bucket_t		*item,
	ion_value_t		value
)
{
	action_status_t status;
	/** Default continue unless value is deleted */
	status.action = action_continue;
	status.err = err_item_not_found;

	if (item->status == IN_USE )	/** if location is not being used, use it */
	{
		/**if it's in use, compare the keys */
		if (hash_map->super.compare(key,item->data,hash_map->super.record.key_size) == IS_EQUAL)
		{
			/** Action function for why the block was brought in */
			lh_write_record_status_in_cache(hash_map,item,DELETED);
			status.err = err_ok;
		}
	}
	return status;
}

err_t
lh_cache_pp(
	linear_hashmap_t	*hash_map,
	int 				cache_number,
	int					bucket_number,
	lh_page_cache_t		**cache
	)
{
#if DEBUG
	io_printf("cache number %i status %i \n",cache_number, hash_map->cache[cache_number].status);
#endif

	*cache 	= &(hash_map->cache[cache_number]);

	int bucket_size = hash_map->record_size * RECORDS_PER_BUCKET;

	if (((*cache)->status == cache_active) && ((*cache)->bucket_idx == bucket_number) )
	{
		return err_ok;												/** The bucket you need is already in the cache
		 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 *	and assumed to be consistent??*/
	}
	else if (((*cache)->status == cache_active) && ((*cache)->bucket_idx != bucket_number))
	{
		/** flush page and fetch new one but do not return memory to system*/
		lh_flush_cache(hash_map,*cache, PRESERVE_CACHE_MEMORY);
	}
	else if ((*cache)->status == cache_invalid)						/** malloc memory for cache */
	{
		(*cache)->cached_bucket = (l_hash_bucket_t *)malloc(bucket_size);
																	/** allocate memory for page */
		if ((*cache)->cached_bucket == NULL)
		{
			return err_out_of_memory;
		}
		(*cache)->status = cache_flushed;							/** cache is allocated but no data yet*/
	}
	/** else the bucket is flushed and just needs new data */
	if (bucket_number != EMPTY_BLOCK_REQUEST)
	{
		fseek(hash_map->file, bucket_number * bucket_size, SEEK_SET);
		fread((*cache)->cached_bucket,bucket_size,1,hash_map->file);
		(*cache)->bucket_idx = bucket_number;
	}
	else
	{
		(*cache)->bucket_idx = EMPTY_BLOCK_REQUEST;
	}
		(*cache)->status = cache_active;							/** cache is now live*/

	return err_ok;													/** @TODO consider error codes on this */
}

err_t
lh_read_cache(
	linear_hashmap_t	*hash_map,
	lh_page_cache_t		*cache,
	int					idx,
	void				**item
)
{
	if (cache->status == cache_invalid)
		return err_uninitialized;				/** trying to read invalid cache */
	/** @TODO - Consider the return time trade off between function call and computation vs code reuse.
	 * If speed is a concern, could be done as inline */
	*item = (l_hash_bucket_t * )(cache->cached_bucket + ( idx * hash_map->record_size));
	return err_ok;
}

int
calculate_cache_location(
	linear_hashmap_t	*hash_map,
	l_hash_bucket_t		*item
)
{
	/** This assumes a contiguous memory cache block allocation
	 * 	which is not the case currently :(
	 */
	/*
	int size_of_cache_page = hash_map->record_size * RECORDS_PER_BUCKET;

	int page_number = (item - hash_map->cache[0].cached_bucket)
	        / size_of_cache_page;
	DUMP(item,"%p");
	DUMP(hash_map->cache[0].cached_bucket,"%p");
	DUMP(size_of_cache_page,"%i");
	DUMP(page_number,"%i");

	* compute with cache the item is in
	return page_number;*/

	int size_of_cache_page = hash_map->record_size * RECORDS_PER_BUCKET;

	int page_number = 0;
	for (;page_number < CACHE_SIZE;page_number++)
	{
		if ((hash_map->cache[page_number].cached_bucket <= item) &&  (item < (hash_map->cache[page_number].cached_bucket+size_of_cache_page)))
			{
			return page_number;
			}
	}
	return -1;  /** Return an invalid cache page*/
}

err_t
lh_write_record_status_in_cache(
	linear_hashmap_t	*hash_map,
	l_hash_bucket_t		*item,
	char				status
)
{
	int page_number = calculate_cache_location(hash_map, item);

	if (status != item->status)							/** Only change is the status is different */
	{
		item->status 							= status;
		hash_map->cache[page_number].status 	= cache_active_written;
	}
	return err_ok;
}

err_t
lh_write_cache_raw(
	linear_hashmap_t	*hash_map,
	lh_page_cache_t		*cache,
	void				*to,
	l_hash_bucket_t		*item
)
{
	/** @TODO Does a check need to be in place to ensure that a user is writing to a valid location in a cache ?*/
	if (cache->status 	== cache_invalid)
	{
		return err_uninitialized;						/** the cache is invalid */
	}

	memcpy(to,item,hash_map->record_size);				/** copy in record */
	cache->status								= cache_active_written;
														/** update status of cache */
	return err_ok;
}

err_t
lh_write_cache_record(
	linear_hashmap_t	*hash_map,
	l_hash_bucket_t		*item,
	ion_key_t			key,
	ion_value_t			value
	)
{
	/** @TODO speed considerations for computation of page vs direct computation ?*/
	/** Determine which cache the item belongs to */
	int page_number = calculate_cache_location(hash_map, item);

	if (hash_map->cache[page_number].status == cache_invalid)
	{
		return err_uninitialized;
	}

	item->status = IN_USE;
	memcpy(item->data, key, (hash_map->super.record.key_size));
	memcpy(item->data + hash_map->super.record.key_size, value,
							(hash_map->super.record.value_size));
	hash_map->cache[page_number].status = cache_active_written;

	return err_ok;
}

err_t
lh_flush_cache(
	linear_hashmap_t	*hash_map,
	lh_page_cache_t		*cache,
	int					action
)
{

	if (cache == NULL)
	{
		return err_item_not_found;
	}
	if (cache->status == cache_invalid)
	{
		return err_ok;			/** @todo this should be a different message */
	}

	int bucket_size = hash_map->record_size * RECORDS_PER_BUCKET;

	/** If the cache has been modified or there is a request for an empty block
	 * 	proceed with writing to disk, otherwise save the I/O and release cache
	 * 	accordingly.
	 */
	if ((cache->status == cache_active_written) || (cache->bucket_idx == EMPTY_BLOCK_REQUEST))
	{
#if DEBUG
		io_printf("flushing to disk\n");
		DUMP(cache->status,"%i");
#endif
		if (cache->bucket_idx != EMPTY_BLOCK_REQUEST)
		{
			fseek(hash_map->file,cache->bucket_idx * bucket_size, SEEK_SET);
		}
		else{
			/** Write new block at end of file */
			fseek(hash_map->file, 0, SEEK_END);											/** and add new bucket */
		}
		/** @FIXME better calculation that direct seek?*/
		/** backup and write back */
		fwrite(cache->cached_bucket,bucket_size,1,hash_map->file);
	}
	if (action == FREE_CACHE_MEMORY)
	{
		free(cache->cached_bucket);
		cache->status = cache_invalid;	/** nothing is in cache */
	}
	else
	{
		cache->status = cache_flushed;		/** set status to flushed */
	}
	return err_ok;
}

return_status_t
lh_action_primary_page(
	linear_hashmap_t		*hash_map,
	lh_page_cache_t			*cache,
	int						bucket,
	ion_key_t				key,
	action_status_t			(*action)(linear_hashmap_t*, ion_key_t, l_hash_bucket_t*, ion_value_t),
	ion_value_t				value

)
{
	/** set number of records touched to 0 */
	return_status_t status;
	status.count = 0;

	/** @TODO initial condition needs to get changed to support active cursor */
	int count 		= 0;

	l_hash_bucket_t * item;

	//check to see if value is in primary bucket
	while (count != RECORDS_PER_BUCKET)
	{
		/** advance through page */
		/*lh_read_cache(hash_map, );*/
		lh_read_cache(hash_map,cache,count,(void*)&item);
#if DEBUG
		DUMP(count * hash_map->record_size,"%i");
#endif
		/** scan through block performing action*/
		action_status_t record_status = action(hash_map, key, item, value);

		status.err = record_status.err;

		if (record_status.err == err_ok) 				/** a record has been touched */
		{
			status.count += 1;
		}
		if (record_status.action == action_exit)		/** if the helper is done, leave */
		{
			return status;
		}
		count++;
	}
	return status;
}

err_t
lh_search_primary_page(
	linear_hashmap_t		*hash_map,
	lh_page_cache_t			*cache,
	lhdict_cursor_t			*cursor /* predicate is in here */
 )
{
	l_hash_bucket_t * item;

#if DEBUG
	DUMP(cursor->record_pntr,"%i");
#endif

	/** check to see if value is in primary bucket */
	while ( cursor->record_pntr != RECORDS_PER_BUCKET)
	{
		/** advance through page */
		lh_read_cache(hash_map, cache, cursor->record_pntr, (void*)&item);

#if DEBUG
		DUMP( cursor->record_pntr * hash_map->record_size,"%i");
#endif
		/** scan through block performing action*/
		if (item->status == EMPTY) 			/** nothing is here, so exit */
		{
							/** in this case, the cursor would be null */
			return err_item_not_found;
		}
		else if (item->status == IN_USE) 		/** if location is not being used, use it */
		{
			/**if it's in use, compare with predicate */
			if (IS_EQUAL == cursor->evaluate_predicate((dict_cursor_t*)cursor,item->data))
			{
				/** and if it satisfies the predicate, retrieve the location */
				return err_ok;
			}
		}
		cursor->record_pntr++;
	}
	return err_not_in_primary_page;
}
