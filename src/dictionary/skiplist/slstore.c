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
	srand(0xDEADBEEF);

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
#endif

	/* TODO malloc error check */
	skiplist->head 			= malloc(sizeof(sl_node_t));
	skiplist->head->next 	= malloc(sizeof(sl_node_t) * skiplist->maxheight);

	skiplist->head->height 	= maxheight - 1;
	skiplist->maxheight 	= maxheight;

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
	return err_ok;
}
