/******************************************************************************/
/**
@file		slstore.c
@author		Kris Wallperington
@brief		Implementation of a Skiplist data store
*/
/******************************************************************************/

#include "slstore.h"

err_t
sl_initialize(
	skiplist_t 	*skiplist,
	key_type_t 	key_type,
	char 		(* compare)(ion_key_t, ion_key_t, ion_key_size_t),
	int 		key_size,
	int 		value_size,
	int 		maxheight,
	int 		pnum,
	int 		pden
)
{
	/* TODO srand may need to be changed */
	srand(time(NULL));

	skiplist->super.key_type 			= key_type;
	skiplist->super.record.key_size 	= key_size;
	skiplist->super.record.value_size 	= value_size;
	skiplist->compare 					= compare;
	skiplist->maxheight 				= maxheight;

	/* TODO potentially check if pden and pnum are invalid (0) */
	skiplist->pden 						= pden;
	skiplist->pnum 						= pnum;

#ifdef DEBUG
	DUMP(skiplist->super.record.key_size, "%d");
	DUMP(skiplist->super.record.value_size, "%d");
	DUMP(skiplist->maxheight, "%d");
	DUMP(skiplist->pnum, "%d");
	DUMP(skiplist->pden, "%d");
	io_printf("%s", "\n");
#endif

	skiplist->head 			= malloc(sizeof(sl_node_t));
	if(NULL == skiplist->head) { return err_out_of_memory; }
	skiplist->head->next 	= malloc(sizeof(sl_node_t) * skiplist->maxheight);
	if(NULL == skiplist->head->next) { return err_out_of_memory; }

	skiplist->head->height 	= maxheight - 1;
	skiplist->head->key 	= NULL;
	skiplist->head->value 	= NULL;

	while(--maxheight >= 0)
	{
		skiplist->head->next[maxheight] = NULL;
	}

	return err_ok;
}

err_t
sl_destroy(
	skiplist_t 	*skiplist
)
{
	sl_node_t 	*cursor 	= skiplist->head,
				*tofree;

	while(cursor != NULL)
	{
		tofree 				= cursor;
		cursor				= cursor->next[0];
		free(tofree->key);
		free(tofree->value);
		free(tofree->next);
		free(tofree);
	}

	skiplist->head			= NULL;

	return err_ok;
}

err_t
sl_insert(
	skiplist_t 		*skiplist,
	ion_key_t 		key,
	ion_value_t 	value
)
{
	/* TODO Should this be refactored to be size_t? */
	int 		key_size 	= skiplist->super.record.key_size;
	int 		value_size 	= skiplist->super.record.value_size;

	sl_node_t 	*newnode 	= malloc(sizeof(sl_node_t));
	if(NULL == newnode) { return err_out_of_memory; }

	newnode->height 		= sl_gen_level(skiplist);
	newnode->next 			= malloc(sizeof(sl_node_t*) * (newnode->height+1));
	if(NULL == newnode->next) { return err_out_of_memory; }

	newnode->key 			= malloc(sizeof(char) * key_size);
	if(NULL == newnode->key) { return err_out_of_memory; }

	newnode->value 			= malloc(sizeof(char) * value_size);
	if(NULL == newnode->value) { return err_out_of_memory; }

	memcpy(newnode->key, key, key_size);
	memcpy(newnode->value, value, value_size);

	sl_node_t 	*cursor 	= skiplist->head;
	sl_level_t 	h;

	for(h = skiplist->head->height; h >= 0; --h)
	{
		//The memcmp will return -1 if key is smaller, 0 if equal, 1 if greater.
		while(NULL != cursor->next[h] &&
					skiplist->compare(key, cursor->next[h]->key, key_size) >= 0)
		{
			cursor = cursor->next[h];
		}

		if(h <= newnode->height)
		{
			newnode->next[h] = cursor->next[h];
			cursor->next[h]  = newnode;
		}
	}

	return err_ok;
}

err_t
sl_query(
	skiplist_t 		*skiplist,
	ion_key_t 		key,
	ion_value_t 	*value
)
{
	/* TODO These should be size_t */
	int 		key_size 	= skiplist->super.record.key_size;
	int 		value_size 	= skiplist->super.record.value_size;
	*value 					= NULL; // Delay initialization
	sl_node_t 	*cursor 	= sl_find_node(skiplist, key);

	if(NULL == cursor->key ||
							skiplist->compare(cursor->key, key, key_size) != 0)
	{
		return err_item_not_found;
	}

	*value 					= malloc(sizeof(char) * value_size);
	if(NULL == value) { return err_out_of_memory; }
	memcpy(*value, cursor->value, value_size);

	return err_ok;
}

err_t
sl_update(
	skiplist_t 		*skiplist,
	ion_key_t 		key,
	ion_value_t 	value
)
{
	/* TODO size_t */
	int 		key_size 	= skiplist->super.record.key_size;
	int 		value_size 	= skiplist->super.record.value_size;
	sl_node_t 	*cursor 	= sl_find_node(skiplist, key);

	/* If the key doesn't exist in the skiplist... */
	if(NULL == cursor->key ||
							skiplist->compare(cursor->key, key, key_size) != 0)
	{
		/* Insert it. TODO Possibly return different error code */
		sl_insert(skiplist, key, value);
		return err_ok;
	}

	/* Otherwise, the key exists and now we have the node to update. */
	memcpy(cursor->value, value, value_size);
	/*TODO Last here, need to write tests. */

	return err_ok;
}

err_t
sl_delete(
	skiplist_t 		*skiplist,
	ion_key_t 		key
)
{
	/* TODO size_t this */
	int 		key_size 	= skiplist->super.record.key_size;
	/* Default return is no item */
	err_t 		status 		= err_item_not_found;

	sl_node_t 	*cursor 	= skiplist->head;
	sl_level_t 	h;

	for(h = skiplist->head->height; h >= 0; --h)
	{
		while(NULL != cursor->next[h] &&
					skiplist->compare(cursor->next[h]->key, key, key_size) < 0)
		{
			cursor 	= cursor->next[h];
		}

		if(NULL != cursor->next[h] &&
					skiplist->compare(cursor->next[h]->key, key, key_size) == 0)
		{
			sl_node_t 			*oldcursor 	= cursor;
			while(NULL != cursor->next[h] &&
					skiplist->compare(cursor->next[h]->key, key, key_size) == 0)
			{
				sl_node_t 		*tofree 	= cursor->next[h];
				sl_node_t 		*relink 	= cursor->next[h];
				sl_level_t 		link_h 		= relink->height;
				while(link_h >= 0)
				{
					while(cursor->next[link_h] != relink)
					{
						cursor = cursor->next[link_h];
					}

					sl_node_t 	*jump 		= relink->next[link_h];
					cursor->next[link_h] 	= jump;
					link_h--;
				}

				free(tofree->key);
				free(tofree->value);
				free(tofree->next);
				free(tofree);

				cursor = oldcursor;
			}

			status = err_ok;
		}
	}

	return status;
}

sl_node_t*
sl_find_node(
	skiplist_t 		*skiplist,
	ion_key_t 		key
)
{
	int 		key_size 	= skiplist->super.record.key_size;
	sl_node_t* 	cursor 		= skiplist->head;
	sl_level_t 	h;

	for(h = skiplist->head->height; h >= 0; h--)
	{
		//The memcmp will return -1 if key is smaller, 0 if equal, 1 if greater.
		while( NULL != cursor->next[h] &&
					skiplist->compare(key, cursor->next[h]->key, key_size) >= 0)
		{
			cursor 			= cursor->next[h];
		}
	}

	return cursor;
}

sl_level_t
sl_gen_level(
	skiplist_t 		*skiplist
)
{
	sl_level_t level = 1;
	while((rand() < skiplist->pnum * (RAND_MAX / skiplist->pden)) &&
													level < skiplist->maxheight)
	{
		level++;
	}

	return level - 1;
}
