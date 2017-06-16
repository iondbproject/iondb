/******************************************************************************/
/**
@file		skip_list.c
@author		Eric Huang
@brief		Implementation of a Skiplist data store
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

#include "skip_list.h"
/* #include "serial_c_iface.h" */

ion_err_t
sl_initialize(
	ion_skiplist_t	*skiplist,
	ion_key_type_t	key_type,
	int				key_size,
	int				value_size,
	int				maxheight,
	int				pnum,
	int				pden
) {
	/* srand(time(NULL)); */

	skiplist->super.key_type			= key_type;
	skiplist->super.record.key_size		= key_size;
	skiplist->super.record.value_size	= value_size;
	skiplist->maxheight					= maxheight;

	skiplist->pden						= pden;
	skiplist->pnum						= pnum;

#if ION_DEBUG
	DUMP(skip_list->super.record.key_size, "%d");
	DUMP(skip_list->super.record.value_size, "%d");
	DUMP(skip_list->maxheight, "%d");
	DUMP(skip_list->pnum, "%d");
	DUMP(skip_list->pden, "%d");
	printf("%s", "\n");
#endif

	skiplist->head = malloc(sizeof(ion_sl_node_t));

	if (NULL == skiplist->head) {
		return err_out_of_memory;
	}

	skiplist->head->next = malloc(sizeof(ion_sl_node_t) * skiplist->maxheight);

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

ion_err_t
sl_destroy(
	ion_skiplist_t *skiplist
) {
	ion_sl_node_t *cursor = skiplist->head, *tofree;

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
	ion_skiplist_t	*skiplist,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_key_size_t		key_size	= skiplist->super.record.key_size;
	ion_value_size_t	value_size	= skiplist->super.record.value_size;

	ion_sl_node_t *newnode			= malloc(sizeof(ion_sl_node_t));

	if (NULL == newnode) {
		return ION_STATUS_ERROR(err_out_of_memory);
	}

	newnode->key = malloc((size_t) key_size);

	if (NULL == newnode->key) {
		free(newnode);
		return ION_STATUS_ERROR(err_out_of_memory);
	}

	newnode->value = malloc((size_t) value_size);

	if (NULL == newnode->value) {
		free(newnode->key);
		free(newnode);
		return ION_STATUS_ERROR(err_out_of_memory);
	}

	memcpy(newnode->key, key, key_size);
	memcpy(newnode->value, value, value_size);

	/* First we check if there's already a duplicate node. If there is, we're
	   going to do a modified insert instead. */
	ion_sl_node_t *duplicate = sl_find_node(skiplist, key);

	if ((NULL != duplicate->key) && (skiplist->super.compare(duplicate->key, key, key_size) == 0)) {
		/* Child duplicate nodes have no height (which is effectively 1). */
		newnode->height = 0;
		newnode->next	= malloc(sizeof(ion_sl_node_t *) * (newnode->height + 1));

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
		newnode->next	= malloc(sizeof(ion_sl_node_t *) * (newnode->height + 1));

		if (NULL == newnode->next) {
			free(newnode->value);
			free(newnode->key);
			free(newnode);
			return ION_STATUS_ERROR(err_out_of_memory);
		}

		ion_sl_node_t	*cursor = skiplist->head;
		ion_sl_level_t	h;

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
sl_get(
	ion_skiplist_t	*skiplist,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_key_size_t		key_size	= skiplist->super.record.key_size;
	ion_value_size_t	value_size	= skiplist->super.record.value_size;
	ion_sl_node_t		*cursor		= sl_find_node(skiplist, key);

	if ((NULL == cursor->key) || (skiplist->super.compare(cursor->key, key, key_size) != 0)) {
		return ION_STATUS_ERROR(err_item_not_found);
	}

	memcpy(value, cursor->value, value_size);

	return ION_STATUS_OK(1);
}

ion_status_t
sl_update(
	ion_skiplist_t	*skiplist,
	ion_key_t		key,
	ion_value_t		value
) {
	ion_status_t status;

	status = ION_STATUS_INITIALIZE;

	ion_key_size_t		key_size	= skiplist->super.record.key_size;
	ion_value_size_t	value_size	= skiplist->super.record.value_size;
	ion_sl_node_t		*cursor		= sl_find_node(skiplist, key);

	/* If the key doesn't exist in the skiplist... */
	if ((NULL == cursor->key) || (skiplist->super.compare(cursor->key, key, key_size) != 0)) {
		/* Insert it. */
		status.error	= sl_insert(skiplist, key, value).error;
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
	ion_skiplist_t	*skiplist,
	ion_key_t		key
) {
	ion_key_size_t key_size = skiplist->super.record.key_size;
	/* Default return is no item */
	ion_status_t status;

	status			= ION_STATUS_INITIALIZE;
	/* If we fall through, then we didn't find what we were looking for. */
	status.error	= err_item_not_found;

	ion_sl_node_t	*cursor = skiplist->head;
	ion_sl_level_t	h;

	for (h = skiplist->head->height; h >= 0; --h) {
		while (NULL != cursor->next[h] && skiplist->super.compare(cursor->next[h]->key, key, key_size) < 0) {
			cursor = cursor->next[h];
		}

		if ((NULL != cursor->next[h]) && (skiplist->super.compare(cursor->next[h]->key, key, key_size) == 0)) {
			ion_sl_node_t *oldcursor = cursor;

			while (NULL != cursor->next[h] && skiplist->super.compare(cursor->next[h]->key, key, key_size) == 0) {
				ion_sl_node_t	*tofree = cursor->next[h];
				ion_sl_node_t	*relink = cursor->next[h];
				ion_sl_level_t	link_h	= relink->height;

				while (link_h >= 0) {
					while (cursor->next[link_h] != relink) {
						cursor = cursor->next[link_h];
					}

					ion_sl_node_t *jump = relink->next[link_h];

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

ion_sl_node_t *
sl_find_node(
	ion_skiplist_t	*skiplist,
	ion_key_t		key
) {
	int				key_size	= skiplist->super.record.key_size;
	ion_sl_node_t	*cursor		= skiplist->head;
	ion_sl_level_t	h;

	for (h = skiplist->head->height; h >= 0; h--) {
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

ion_sl_level_t
sl_gen_level(
	ion_skiplist_t *skiplist
) {
	ion_sl_level_t level = 1;

	while ((rand() < skiplist->pnum * (RAND_MAX / skiplist->pden)) && level < skiplist->maxheight) {
		level++;
	}

	return level - 1;
}

void
print_skiplist(
	ion_skiplist_t *skiplist
) {
	ion_sl_node_t *cursor = skiplist->head;

	while (NULL != cursor->next[0]) {
		ion_sl_level_t level = cursor->next[0]->height + 1;

		if (key_type_numeric_signed == skiplist->super.key_type) {
			int key		= *((int *) cursor->next[0]->key);
			int value	= *(int *) cursor->next[0]->value;

			printf("k: '%d' (v: '%d') [l: %d] -- ", key, value, level);
		}
		else if (key_type_null_terminated_string == skiplist->super.key_type) {
			char	*key	= cursor->next[0]->key;
			int		value	= *(int *) cursor->next[0]->value;

			printf("k: '%s' (v: '%d') [l: %d] -- ", key, value, level);
		}

		cursor = cursor->next[0];
	}

	printf("%s", "END\n\n");
}
