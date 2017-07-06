/******************************************************************************/
/**
@file		skip_list_types.h
@author		Eric Huang
@brief		Contains all types local to the skiplist data structure
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
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
