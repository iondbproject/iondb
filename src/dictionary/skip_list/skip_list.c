/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Implementation of a Skiplist data store
*/
/******************************************************************************/

#include "skip_list.h"
/* #include "serial_c_iface.h" */

err_t
sl_initialize(
	skiplist_t	*skiplist,
	key_type_t	key_type,
	int			key_size,
	int			value_size,
	int			maxheight,
	int			pnum,
	int			pden
) {
	/* TODO srand may need to be changed */
	/* srand(time(NULL)); */

	skiplist->super.key_type			= key_type;
	skiplist->super.record.key_size		= key_size;
	skiplist->super.record.value_size	= value_size;
	skiplist->maxheight					= maxheight;

	/* TODO potentially check if pden and pnum are invalid (0) */
	skiplist->pden						= pden;
	skiplist->pnum						= pnum;

#if DEBUG
	DUMP(skip_list->super.record.key_size, "%d");
	DUMP(skip_list->super.record.value_size, "%d");
	DUMP(skip_list->maxheight, "%d");
	DUMP(skip_list->pnum, "%d");
	DUMP(skip_list->pden, "%d");
	io_printf("%s", "\n");
#endif

	skiplist->head = malloc(sizeof(sl_node_t));

	if (NULL == skiplist->head) {
		return err_out_of_memory;
	}

	skiplist->head->next = malloc(sizeof(sl_node_t) * skiplist->maxheight);

	if (NULL == skiplist->head->next) {
		free(skiplist->head);
		skiplist->head = NULL;
		return err_out_of_memory;
	}

	skiplist->head->height	= maxheight - 1;
	skiplist->head->key		= NULL;
	skiplist->head->value	= NULL;

	while (--maxheight >= 0) {
		skiplist->head->next[maxheight] = NULL;
	}

	return err_ok;
}

err_t
sl_destroy(
	skiplist_t *skiplist
) {
	sl_node_t *cursor = skiplist->head, *tofree;

	while (cursor != NULL) {
		tofree	= cursor;
		cursor	= cursor->next[0];
		free(tofree->key);
		free(tofree->value);
		free(tofree->next);
		free(tofree);
	}

	skiplist->head = NULL;

	return err_ok;
}

ion_status_t
sl_insert(
	skiplist_t	*skiplist,
	ion_key_t	key,
	ion_value_t value
) {
	/* TODO Should this be refactored to be size_t? */
	int key_size		= skiplist->super.record.key_size;
	int value_size		= skiplist->super.record.value_size;

	sl_node_t *newnode	= malloc(sizeof(sl_node_t));

	if (NULL == newnode) {
		return ION_STATUS_ERROR(err_out_of_memory);
	}

	newnode->key = malloc(key_size);

	if (NULL == newnode->key) {
		free(newnode);
		return ION_STATUS_ERROR(err_out_of_memory);
	}

	newnode->value = malloc(value_size);

	if (NULL == newnode->value) {
		free(newnode->key);
		free(newnode);
		return ION_STATUS_ERROR(err_out_of_memory);
	}

	memcpy(newnode->key, key, key_size);
	memcpy(newnode->value, value, value_size);

	/* First we check if there's already a duplicate node. If there is, we're
	 * going to do a modified insert instead. TODO write unit tests to check this
	 */
	sl_node_t *duplicate = sl_find_node(skiplist, key);

	if ((NULL != duplicate->key) && (skiplist->super.compare(duplicate->key, key, key_size) == 0)) {
		/* Child duplicate nodes have no height (which is effectively 1). */
		newnode->height = 0;
		newnode->next	= malloc(sizeof(sl_node_t *) * (newnode->height + 1));

		if (NULL == newnode->next) {
			free(newnode->value);
			free(newnode->key);
			free(newnode);
			return ION_STATUS_ERROR(err_out_of_memory);
		}

		/* We want duplicate to be the last node in the block of duplicate
		 * nodes, so we traverse along the bottom until we get there.
		 */
		while (NULL != duplicate->next[0] && skiplist->super.compare(duplicate->next[0]->key, key, key_size) == 0) {
			duplicate = duplicate->next[0];
		}

		/* Only one height to worry about */
		newnode->next[0]	= duplicate->next[0];
		duplicate->next[0]	= newnode;
	}
	else {
		/* If there's no duplicate node, we do a vanilla insert instead */
		newnode->height = sl_gen_level(skiplist);
		newnode->next	= malloc(sizeof(sl_node_t *) * (newnode->height + 1));

		if (NULL == newnode->next) {
			free(newnode->value);
			free(newnode->key);
			free(newnode);
			return ION_STATUS_ERROR(err_out_of_memory);
		}

		sl_node_t	*cursor = skiplist->head;
		sl_level_t	h;

		for (h = skiplist->head->height; h >= 0; --h) {
			/* The memcmp will return -1 if key is smaller, 0 if equal, 1 if greater. */
			while (NULL != cursor->next[h] && skiplist->super.compare(key, cursor->next[h]->key, key_size) >= 0) {
				cursor = cursor->next[h];
			}

			if (h <= newnode->height) {
				newnode->next[h]	= cursor->next[h];
				cursor->next[h]		= newnode;
			}
		}
	}

	return ION_STATUS_OK(1);
}

ion_status_t
sl_query(
	skiplist_t	*skiplist,
	ion_key_t	key,
	ion_value_t value
) {
	/* TODO These should be size_t */
	int			key_size	= skiplist->super.record.key_size;
	int			value_size	= skiplist->super.record.value_size;
	sl_node_t	*cursor		= sl_find_node(skiplist, key);

	if ((NULL == cursor->key) || (skiplist->super.compare(cursor->key, key, key_size) != 0)) {
		return ION_STATUS_ERROR(err_item_not_found);
	}

	memcpy(value, cursor->value, value_size);

	return ION_STATUS_OK(1);
}

ion_status_t
sl_update(
	skiplist_t	*skiplist,
	ion_key_t	key,
	ion_value_t value
) {
	ion_status_t status;

	status = ION_STATUS_INITIALIZE;

	/* TODO size_t */
	int			key_size	= skiplist->super.record.key_size;
	int			value_size	= skiplist->super.record.value_size;
	sl_node_t	*cursor		= sl_find_node(skiplist, key);

	/* If the key doesn't exist in the skip_list... */
	if ((NULL == cursor->key) || (skiplist->super.compare(cursor->key, key, key_size) != 0)) {
		/* Insert it. TODO Possibly return different error code */
		sl_insert(skiplist, key, value);
		status.error	= err_ok;
		status.count	= 1;
		return status;
	}

	/* Otherwise, the key exists and now we have the node to update. */

	/* While the cursor still has the same key as the target key... */
	while (NULL != cursor && skiplist->super.compare(cursor->key, key, skiplist->super.record.key_size) == 0) {
		/* Update the value, and then move on to the next node. */
		memcpy(cursor->value, value, value_size);
		cursor = cursor->next[0];
		status.count++;
	}

	status.error = err_ok;
	return status;
}

ion_status_t
sl_delete(
	skiplist_t	*skiplist,
	ion_key_t	key
) {
	/* TODO size_t this */
	int key_size = skiplist->super.record.key_size;
	/* Default return is no item */
	ion_status_t status;

	status			= ION_STATUS_INITIALIZE;
	/* If we fall through, then we didn't find what we were looking for. */
	status.error	= err_item_not_found;

	sl_node_t	*cursor = skiplist->head;
	sl_level_t	h;

	for (h = skiplist->head->height; h >= 0; --h) {
		while (NULL != cursor->next[h] && skiplist->super.compare(cursor->next[h]->key, key, key_size) < 0) {
			cursor = cursor->next[h];
		}

		if ((NULL != cursor->next[h]) && (skiplist->super.compare(cursor->next[h]->key, key, key_size) == 0)) {
			sl_node_t *oldcursor = cursor;

			while (NULL != cursor->next[h] && skiplist->super.compare(cursor->next[h]->key, key, key_size) == 0) {
				sl_node_t	*tofree = cursor->next[h];
				sl_node_t	*relink = cursor->next[h];
				sl_level_t	link_h	= relink->height;

				while (link_h >= 0) {
					while (cursor->next[link_h] != relink) {
						cursor = cursor->next[link_h];
					}

					sl_node_t *jump = relink->next[link_h];

					cursor->next[link_h] = jump;
					link_h--;
				}

				free(tofree->key);
				free(tofree->value);
				free(tofree->next);
				free(tofree);

				cursor = oldcursor;
				status.count++;
			}

			status.error = err_ok;
		}
	}

	return status;
}

sl_node_t *
sl_find_node(
	skiplist_t	*skiplist,
	ion_key_t	key
) {
	int			key_size	= skiplist->super.record.key_size;
	sl_node_t	*cursor		= skiplist->head;
	sl_level_t	h;

	for (h = skiplist->head->height; h >= 0; h--) {
		/* TODO Step through this and verify its integrity for all cases */
		while (NULL != cursor->next[h] && skiplist->super.compare(cursor->next[h]->key, key, key_size) <= 0) {
			if ((NULL != cursor->next[h]) && (skiplist->super.compare(cursor->next[h]->key, key, key_size) == 0)) {
				return cursor->next[h];
			}

			cursor = cursor->next[h];
		}
	}

	/* Key was not found, so return closest thing to that key */
	return cursor;
}

sl_level_t
sl_gen_level(
	skiplist_t *skiplist
) {
	sl_level_t level = 1;

	while ((rand() < skiplist->pnum * (RAND_MAX / skiplist->pden)) && level < skiplist->maxheight) {
		level++;
	}

	return level - 1;
}

void
print_skiplist(
	skiplist_t *skiplist
) {
	sl_node_t *cursor = skiplist->head;

	while (NULL != cursor->next[0]) {
		int			key		= *((int *) cursor->next[0]->key);
		char		*value	= (char *) cursor->next[0]->value;
		sl_level_t	level	= cursor->next[0]->height + 1;

		printf("k: %d (v: %s) [l: %d] -- ", key, value, level);
		cursor = cursor->next[0];
	}

	printf("%s", "END\n\n");
}
