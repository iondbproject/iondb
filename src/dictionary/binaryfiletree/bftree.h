#ifndef BFTREE_H
#define BFTREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../dictionary/dicttypes.h"
#include "./../dictionary.h"
#include "./../filehandler.h"
#include "./../../kv_system.h"
#include "./../../io.h"

#define EMPTY 			-1
#define DELETED 		-2
#define IN_USE 			-3

#define size_of_key  	4
#define size_of_value 	10

typedef struct node
{
	int 	parent;
	int 	left;
	int 	right;
	int 	status;
	int 	key;
	int 	value;			/**< offset into data file */
} node_t;

typedef struct bft_file_tree
{
	dictionary_parent_t		super;
	FILE 					*key_file_ptr;		/**< Pointer to key file store */
	FILE					*value_file_ptr;	/**
	//fpos_t 					start_of_data;		/**< indicates start of data block */
	write_concern_t 		write_concern;		/**< The current @p write_concern level
	 	 	 	 	 	 	 	 	 	 	 	 	 of the file*/
} bft_file_tree_t;

err_t
bft_insert(
	FILE 		*key_file,
	FILE 		*value_file,
	int 		key,
	ion_value_t value
);

err_t
bft_find(
	FILE 		*key_file,
	FILE 		*value_file,
	int 		key,
	ion_value_t value
);

err_t
bft_delete(
	FILE 		*file,
	int 		key
);

err_t
bft_initialize(
		bft_file_tree_t 	binary_tree,
	    key_type_t			key_type,
		ion_key_size_t		key_size,
		ion_value_size_t	value_size
);

err_t
bft_update(
	FILE 		*key_file,
	FILE 		*value_file,
	int 		key,
	ion_value_t value
);

/**
@brief		Destroys the binary tree files

@details	Destroys the files and frees the underlying memory.

@param		binary_tree
				The binary tree to be destroyed.
@return		The status describing the result of the destruction
*/
err_t
bft_destroy(
		bft_file_tree_t 	*binary_tree
);
#ifdef __cplusplus
}
#endif

#endif
