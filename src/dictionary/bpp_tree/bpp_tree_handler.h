/******************************************************************************/
/**
@file
@author		Graeme Douglas
@brief		The handler for a B+ tree.
*/
/******************************************************************************/

#if !defined(BPP_TREE_HANDLER_H_)
#define BPP_TREE_HANDLER_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../dictionary_types.h"
#include "./../dictionary.h"
#include "../../key_value/kv_system.h"
#include "../../file/linked_file_bag.h"
#include "bpp_tree.h"

typedef struct bplusplustree {
	ion_dictionary_parent_t super;
	/*ion_bpp_handle_t*/ion_bpp*		tree;
	/*ion_lfb_t				values;*/ //OLD
} ion_bpptree_t;

typedef struct {
	ion_dict_cursor_t	super;		/**< Supertype of cursor		*/
	ion_key_t			cur_key;/**< Current key we're visiting */
	ion_file_offset_t	offset;		/**< offset in LFB; holds value */
} ion_bpp_cursor_t;

/**
@brief		Registers a specific handler for a  dictionary instance.

@details	Registers functions for handlers.  This only needs to be called
			once for each type of dictionary that is present.

@param	  handler
				The handler for the dictionary instance that is to be
				initialized.
*/
void
bpptree_init(
	ion_dictionary_handler_t *handler
);

#if defined(__cplusplus)
}
#endif

#endif /* BPP_TREE_HANDLER_H_ */
