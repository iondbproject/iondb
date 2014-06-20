/******************************************************************************/
/**
@file		slstore.h
@author		Kris Wallperington
@brief		Implementation of a Skiplist data store
*/
/******************************************************************************/
#ifndef SLSTORE_H
#define SLSTORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "./../../io.h"
#include "./../../kv_system.h"

typedef int sl_level_t;

/**
@brief 	Struct of a node in the skiplist.
 */
typedef struct sl_node
{
	ion_key_t		key;
	ion_value_t		value;
	sl_level_t		height;
	struct sl_node	**next;
} sl_node_t;

/**
@brief 	Struct of the Skiplist, holds metadata and the entry point
 		into the skiplist.
 */
typedef struct skiplist
{
	sl_node_t	*head;
	sl_level_t	maxheight;
	int			key_size;
	int			value_size;
	int			pnum;
	int			pden;
} skiplist_t;


#ifdef __cplusplus
}
#endif

#endif /* SLSTORE_H */
