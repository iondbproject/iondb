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
	int 		key_size,
	int 		value_size,
	int 		maxheight,
	int 		pnum,
	int 		pden
)
{

	/* TODO srand may need to be changed */
	srand(time(NULL));

	skiplist->key_size 		= key_size;
	skiplist->value_size 	= value_size;
	skiplist->maxheight 	= maxheight;

	/* TODO potentially check if pden and pnum are invalid (0) */
	skiplist->pden 			= pden;
	skiplist->pnum 			= pnum;

#ifdef DEBUG
	DUMP(skiplist->key_size, "%d");
	DUMP(skiplist->value_size, "%d");
	DUMP(skiplist->maxheight, "%d");
	DUMP(skiplist->pnum, "%d");
	DUMP(skiplist->pden, "%d");
	printf("%s", "\n");
#endif

	/* TODO malloc error check */
	skiplist->head 			= malloc(sizeof(sl_node_t));
	skiplist->head->next 	= malloc(sizeof(sl_node_t) * skiplist->maxheight);

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
	sl_node_t *cursor = skiplist->head, *tofree;

	while(cursor != NULL)
	{
		tofree = cursor;
		cursor = cursor->next[0];
		free(tofree->key);
		free(tofree->value);
		free(tofree->next);
		free(tofree);
	}

	skiplist->head = NULL;

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
	int 		key_size 	= skiplist->key_size;
	int 		value_size 	= skiplist->value_size;

	sl_node_t 	*newnode 	= malloc(sizeof(sl_node_t));
	newnode->height 		= sl_gen_level(skiplist);
	newnode->next 			= malloc(sizeof(sl_node_t*) * (newnode->height+1));

	newnode->key 			= malloc(sizeof(char) * key_size);
	newnode->value 			= malloc(sizeof(char) * value_size);
	memcpy(newnode->key, key, key_size);
	memcpy(newnode->value, value, value_size);

	io_printf("This is our key: %d\n", (int) *newnode->key);

	sl_node_t 	*cursor 	= skiplist->head;
	sl_level_t 	h;

	for(h = skiplist->head->height; h >= 0; --h)
	{
		//The memcmp will return -1 if key is smaller, 0 if equal, 1 if greater.
		while(NULL != cursor->next[h] &&
							memcmp(key, cursor->next[h]->key, key_size) >= 0)
		{
			cursor = cursor->next[h];
		}

		if(h <= newnode->height)
		{
			newnode->next[h] 	= cursor->next[h];
			cursor->next[h] 	= newnode;
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
	int 	value_size 	= skiplist->value_size;
	*value 				= malloc(sizeof(char) * value_size);
	/* TODO last here */

	return err_ok;
}

sl_node_t*
sl_find_node(
	skiplist_t 		*skiplist,
	ion_key_t 		key
)
{
	int 		key_size 	= skiplist->key_size;
	sl_node_t* 	cursor 		= skiplist->head;
	sl_level_t 	h;

	for(h = skiplist->head->height; h >= 0; h--)
	{
		//The memcmp will return -1 if key is smaller, 0 if equal, 1 if greater.
		while( NULL != cursor->next[h] &&
							memcmp(key, cursor->next[h]->key, key_size) >= 0)
		{
			cursor = cursor->next[h];
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
