/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Contains all types local to the skiplist data structure
*/
/******************************************************************************/

#if !defined(SKIP_LIST_TYPES_H_)
#define SKIP_LIST_TYPES_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary_types.h"
#include "./../dictionary.h"

#include "../../key_value/kv_system.h"

typedef int ion_sl_level_t;	/**< Height of a skiplist */

/**
@brief  Struct of a node in the skiplist.
*/
typedef struct sl_node {
	ion_key_t		key;		/**< Key of a skiplist node */
	ion_value_t		value;		/**< Value of a skiplist node */
	ion_sl_level_t	height;			/**< Height index of a skiplist node
									 (counts from 0) */
	struct sl_node	**next;		/**< Array of nodes that form the next
									 column in the skiplist */
} ion_sl_node_t;

/**
@brief  Struct of the Skiplist, holds metadata and the entry point
		into the skiplist.
*/
typedef struct skiplist {
	/* TODO yuck */
	ion_dictionary_parent_t super;	/**< Parent structure holding dictionary level
									information */
	ion_sl_node_t			*head;	/**< Entry point into the skiplist. Does not hold
									any key/value information */
	ion_sl_level_t			maxheight;	/**< Maximum height of the skiplist in terms of
										the number of nodes */
	int						pnum;	/**< Probability NUMerator, used in height gen */
	int						pden;	/**< Probability DENominator, used in height gen */
} ion_skiplist_t;

typedef struct
	sldict_cursor {
	ion_dict_cursor_t	super;			/**< Supertype of cursor */
	ion_sl_node_t		*current;		/**< Current visited spot */
} ion_sldict_cursor_t;

#if defined(__cplusplus)
}
#endif

#endif /* SKIP_LIST_TYPES_H_ */
