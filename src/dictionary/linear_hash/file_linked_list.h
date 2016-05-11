/*
 * file_linked_list.h
 *
 *  Created on: Jun 8, 2015
 *      Author: workstation
 */

#ifndef FILE_LINKED_LIST_H_
#define FILE_LINKED_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "../dictionary_types.h"
#include "./../dictionary.h"
#include "../../util/id_encoder/file_encoder.h"

#include "./../../kv_system.h"
#include "./../../kv_io.h"


#define LOW			0
#define MEDIUM		1
#define HIGH		2
#define DEBUG_LEVEL	LOW


#define END_OF_LIST 	-1
#define HEAD_NODE 		0
#define INVALID_NODE	-2

/**
 * Tracks the state of the iterator
 */
typedef enum iterator_status {
	uninitialized,							/**< uninitialized - the iterator is not usable and needs to be initialized*/
	initialized,  							/**< initialized - the iterator has been initialized but has not been consumed*/
	active,        							/**< active - the iterator is active in traversal of the list*/
	removed									/**< the current node has been removed so previous is still previous when advancing */
} iterator_status_t;

/**
@brief		Prototype declaration for hashmap
 */
typedef struct ll_file 	ll_file_t;

/**
@brief		Struct used to maintain individual records in the hashmap.
*/
typedef struct ll_file_node
{
	int 				next;			/**< next node in list*/
	unsigned char 		data[];			/**< the data in the bucket */
} ll_file_node_t;

/**
@brief		Struct used to maintain an instance of an in memory hashmap.
*/
struct ll_file
{
	/** @todo find a better way to represent position */
	int					previous;			/**< position of previous node */
	int					current;			/**< current position (node) in file */
	int 				next;				/**< next node position in file */
	iterator_status_t 	iterator_state;		/**< tracks current state of a single iterator */
	int	 				(*compare)(ll_file_t *, ll_file_node_t *, ll_file_node_t *);
											/**< Comparison function for nodes */
	FILE				*file;				/**< file pointer */
	char				*file_name;			/**< file name */
	int					node_size;			/**<size of node */
};

/**
@brief		This function initializes a linearu in memory hash map.

@param		hashmap
				Pointer to the hashmap instance to initialize.
@param		key_type
				The type of key that is being stored in the collection.
@param 		key_size
				The size of the key in bytes.
@param		value_size
				The size of the value in bytes.
@param		size
				The size of the hashmap in item
				(@p key_size + @p value_size + @c 1)
@return		The status describing the result of the initialization.
 */
err_t
fll_create(
		ll_file_t			*linked_list_file,
	    int	 				(*compare)(ll_file_t *, ll_file_node_t *, ll_file_node_t *),
		key_type_t			key_type,
		ion_key_size_t		key_size,
		ion_value_size_t	value_size,
		int					bucket,
		int 				id
);

/**
 * Reopens an existing file
 * @param linked_list_file
 * @param compare
 * @param key_type
 * @param key_size
 * @param value_size
 * @param bucket
 * @return
 */
err_t
fll_open
(
	ll_file_t			*linked_list_file,
	int	 				(*compare)(ll_file_t *, ll_file_node_t *, ll_file_node_t*),
	key_type_t			key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size,
	int					bucket,
	int					id
);

/**
 * Closes file and frees resources but does not delete the file from disk
 * @param linked_list_file
 * @return
 */
err_t
fll_close(
	ll_file_t		*linked_list_file
);

/**
 * returns the next node in the list. Assumes that the
 * head node is the first node in the file.
 * It will be an empty node that is used to anchor the list.
 * @param linked_list_file
 * 			The linked list of operate upon
 * @param ll_node
 * 			node pointer to which node will be returned
 * @return
 * 			The status if a another node is available
 */
err_t
fll_next(
	ll_file_t				*linked_list_file,
	volatile ll_file_node_t			*ll_node
);

/**
 * Insert a node into the ll
 * @param linked_list_file
 * @param node
 * 			The node to insert
 * @return
 * 			The status of the insert
 */
err_t
fll_insert(
	ll_file_t				*linked_list_file,
	ll_file_node_t			*node
);

/**
 * Delete a node from the list, based on something
 * @param linked_list_file
 * @param ion_key_t
 * 			The key to delete from the list
 * @return
 */
err_t
fll_delete(
	ll_file_t				*linked_list_file,
	ion_key_t				key
);

/**
 * Finds a node
 * @param linked_list_file
 * @param key
 * @param node
 * @return
 */
err_t
fll_find(
	ll_file_t				*linked_list_file,
	ion_key_t				key,
	ll_file_node_t			*ll_node
);

/**
 * Returns the node at the current position
 * @param linked_list_file
 * @param node
 * @return
 */
err_t
fll_get(
	ll_file_t				*linked_list_file,
	volatile ll_file_node_t			*ll_node
);


/**
 * Create a node that is contiguous in memory
 * @param linked_list_file
 * @param record
 * @param key
 * @param value
 * @param node
 * @return
 */
err_t
fll_create_node(
	ll_file_t 				*linked_list_file,
	record_info_t 			*record,
	ion_key_t				key,
	ion_value_t				value,
	ll_file_node_t			**node
	);


/**
 * Compares two nodes. If base_node is less than compare_node, it will return a negative value.
 * If base_node is equal to compare_node, it will return zero (0).
 * If base_node is greater than compare_node, it will return a positive value.
 * @param linked_list_file
 * 			The linked list compare is associated with
 * @param base_node
 * 			The node that is being compared as reference
 * @param compare_node
 * 			The node that is being compared against
 * @return
 * 			The state of comparison
 */
int
fll_compare(
	ll_file_t 				*linked_list_file,
	ll_file_node_t 			*base_node,
	ll_file_node_t			*compare_node
);



long int
fll_write_node(
	ll_file_t					*linked_list_file,
	ll_file_node_t				*head_node
);

err_t
fll_update_node(
	ll_file_t					*linked_list_file,
	ll_file_node_t				*ll_probe,
	long int					ll_probe_pos
);

/**
 * @brief		Updates the node at the position currently being pointed to.
 *
 * @param 		linked_list_file
 * 					The file to update.
 * @param 		ll_probe
 * 					The node that is being updated.
 * 					Do not change key value.
 * @return
 */
err_t
fll_update(
	ll_file_t					*linked_list_file,
	ll_file_node_t				*ll_probe
);

/**
 * Resets iterator to the start of list.
 * @param linked_list_file
 * 			The linked list being operated on
 * @return
 */
err_t
fll_reset(
	ll_file_t					*linked_list_file
);

/**
 * Removes the note at the current position.  Must be used in conjunction with next operator and removes item
 * that was returned by next.
 * @param linked_list_file
 * 			The linked list of operate on
 * @return
 */
err_t
fll_remove(
	ll_file_t					*linked_list_file
);

#ifdef __cplusplus
}
#endif

#endif /* FILE_LL_H_ */
