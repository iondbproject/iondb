/******************************************************************************/
/**
@file		skip_list_types.c
@author		Kris Wallperington
@brief		Contains all types local to the skip_list data structure
*/
/******************************************************************************/

#if !defined(SKIP_LIST_TYPES_H_)
#define SKIP_LIST_TYPES_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary_types.h"
#include "./../dictionary.h"

#include "./../../kv_io.h"
#include "./../../kv_system.h"

typedef int sl_level_t;	/**< Height of a skip_list */

/**
@brief  Struct of a node in the skip_list.
*/
typedef struct sl_node {
	ion_key_t		key;		/**< Key of a skip_list node */
	ion_value_t		value;		/**< Value of a skip_list node */
	sl_level_t		height;		/**< Height index of a skip_list node
									 (counts from 0) */
	struct sl_node	**next;		/**< Array of nodes that form the next
									 column in the skip_list */
} sl_node_t;

/**
@brief  Struct of the Skiplist, holds metadata and the entry point
		into the skip_list.
*/
typedef struct skiplist {
	/* TODO yuck */
	dictionary_parent_t super;	/**< Parent structure holding dictionary level
								 information */
	sl_node_t			*head;	/**< Entry point into the skip_list. Does not hold
								 any key/value information */
	sl_level_t			maxheight;	/**< Maximum height of the skip_list in terms of
									the number of nodes */
	int					pnum;	/**< Probability NUMerator, used in height gen */
	int					pden;	/**< Probability DENominator, used in height gen */
} skiplist_t;

typedef struct sldict_cursor {
	dict_cursor_t	super;			/**< Supertype of cursor */
	sl_node_t		*current;		/**< Current visited spot */
} sldict_cursor_t;

#if defined(__cplusplus)
}
#endif

#endif /* SKIP_LIST_TYPES_H_ */
