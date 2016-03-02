/******************************************************************************/
/**
@file		sltypes.c
@author		Kris Wallperington
@brief		Contains all types local to the skiplist data structure
*/
/******************************************************************************/

#ifndef SLTYPES_H
#define SLTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "./../dicttypes.h"
#include "./../dictionary.h"

#include "./../../kv_io.h"
#include "./../../kv_system.h"

typedef int sl_level_t; /**< Height of a skiplist */

/**
@brief 	Struct of a node in the skiplist.
 */
typedef struct sl_node
{
	ion_key_t		key; 		/**< Key of a skiplist node */
	ion_value_t		value;		/**< Value of a skiplist node */
	sl_level_t		height;		/**< Height index of a skiplist node
									 (counts from 0) */
	struct sl_node	**next; 	/**< Array of nodes that form the next
	 	 	 	 	 	 	 	     column in the skiplist */
} sl_node_t;

/**
@brief 	Struct of the Skiplist, holds metadata and the entry point
 		into the skiplist.
 */
typedef struct skiplist
{
	/* TODO yuck */
	dictionary_parent_t
				super; 		/**< Parent structure holding dictionary level
							     information */
	sl_node_t	*head; 		/**< Entry point into the skiplist. Does not hold
	 	 	 	 	 	 	     any key/value information */
	sl_level_t	maxheight;	/**< Maximum height of the skiplist in terms of
	 	 	 	 	 	 	     the number of nodes */
	int			pnum;		/**< Probability NUMerator, used in height gen */
	int			pden;		/**< Probability DENominator, used in height gen */
} skiplist_t;

typedef struct sldict_cursor
{
	dict_cursor_t 		super; 		/**< Supertype of cursor */
	sl_node_t 			*current; 	/**< Current visited spot */
} sldict_cursor_t;


#ifdef __cplusplus
}
#endif

#endif /* SLTYPES_H */
