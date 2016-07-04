/*
 * file_linked_list.c
 *
 *  Created on: Jun 8, 2015
 *	  Author: workstation
 */

#include "file_linked_list.h"

err_t
fll_reset(
	ll_file_t *linked_list_file
) {
	ll_file_node_t *ll_probe;	/*allocate ll_probe node to access head */

	ll_probe							= (ll_file_node_t *) malloc(linked_list_file->node_size);
	linked_list_file->current			= HEAD_NODE;						/*point to head node in list and clear rest of pointers*/
	linked_list_file->previous			= INVALID_NODE;
	linked_list_file->iterator_state	= initialized;						/*the iterator is initialized and ready to consume nodes */
	fll_get(linked_list_file, ll_probe);/*Read in the head node and determine what to do*/
	linked_list_file->next				= ll_probe->next;					/*Point to the first node in the list */
	free(ll_probe);
	return err_ok;	/*@todo cleanup return codes */
}

err_t
fll_open(
	ll_file_t *linked_list_file,
	int (*compare)(ll_file_t *, ll_file_node_t *, ll_file_node_t *),
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	int bucket,
	int id
) {
	fe_filename_t filename;

	filename.instance_id			= id;			/*This is the parent id */
	filename.type.child.child_id	= bucket;
	fe_encode_child_id(&filename);
	/* allocation space for file name */
	/* io_printf("%i",FILENAME_SIZE); */
	/* io_printf("%p\n",linked_list_file->file_name); */
	/* if(linked_list_file->file_name == NULL) {io_printf("NULL\n");} */
	/* free(linked_list_file->file_name); */
	linked_list_file->file_name = malloc(FILENAME_SIZE);/* malloc(strlen(filename.child.child_filename)+1 * sizeof(char));//(char*)malloc(FILENAME_SIZE); */
	strcpy(linked_list_file->file_name, filename.type.child.child_filename);
	filename.destroy(&filename);

	/* attempt to open the file */
	linked_list_file->file = fopen(linked_list_file->file_name, "r+b");

	/*NOTE: On windows machines file !!MUST!! be opened with +b to allow for binary mode,*/
	if (linked_list_file->file == NULL) {
		/*then the file did not exist, so create it!*/
#if DEBUG
		io_printf("file does note exist!\n");
#endif
		free(linked_list_file->file_name);	/*free up before you call again*/
		linked_list_file->file_name = NULL;
		return err_item_not_found;
	}
	else {
#if DEBUG
		io_printf("file already exists!\n");
#endif
		linked_list_file->node_size = sizeof(((ll_file_node_t *) 0)->next) + key_size + value_size;
		linked_list_file->compare	= compare;
		return fll_reset(linked_list_file);	/*and reset */
	}
}

err_t
fll_create(
	ll_file_t *linked_list_file,
	int (*compare)(ll_file_t *, ll_file_node_t *, ll_file_node_t *),
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	int bucket,
	int id
) {
	/*This assumes that the the file may or may not exist and
	 * if it does, it will delete and create a new file based linked
	 * list
	 */

	fe_filename_t filename;

	filename.instance_id			= id;			/*This is the parent id */
	filename.type.child.child_id	= bucket;
	fe_encode_child_id(&filename);
	/* allocation space for file name */
	linked_list_file->file_name		= (char *) malloc(FILENAME_SIZE);
	strcpy(linked_list_file->file_name, filename.type.child.child_filename);/* VALGRIND HATES THIS --Heath */
	filename.destroy(&filename);

	linked_list_file->file = fopen(linked_list_file->file_name, "w+b");

	/*NOTE: On windows machines file !!MUST!! be opened with +b to allow for binary mode,
									otherwise when 0x0A is encountered 0x0D will be included (\r\n)*/
	/* create head node and update */
	ll_file_node_t *head_node = malloc(sizeof(*head_node) + key_size + value_size);

/*	ll_file_node_t *head_node = malloc(sizeof(head_node->next) + key_size + value_size); */

	head_node->next						= END_OF_LIST;
	linked_list_file->node_size			= sizeof(*head_node) + key_size + value_size;
/*	linked_list_file->node_size			= sizeof(head_node->next) + key_size + value_size; */
	/* data is garbage so just leave it */
	linked_list_file->current			= HEAD_NODE;
	linked_list_file->next				= INVALID_NODE;
	linked_list_file->next				= head_node->next;
	linked_list_file->iterator_state	= uninitialized;

	/* write out head node */
	fll_write_node(linked_list_file, head_node);

	/* compute node size */
	linked_list_file->node_size = sizeof(*head_node) + key_size + value_size;
/*	linked_list_file->node_size = key_size + value_size + sizeof(head_node->next); */
	/*clean up to handle general data with a pointer */

	/* bind comparator */
	linked_list_file->compare = compare;

	free(head_node);
	/*@todo fix return value */
	return 0;
}

err_t
fll_next(
	/* HEATH //When this method is called, something seems to overflow; cannot see what though. */
	ll_file_t				*linked_list_file,
	volatile ll_file_node_t *ll_node
) {
	/*@todo Make sure to check status of next!!! */
	if (linked_list_file->iterator_state == uninitialized) {
		/*check the state of the iterator before proceeding */
		return err_uninitialized;
	}
	else {
		if (linked_list_file->iterator_state != removed) {
			/*if the current item has been removed, previous stays as previous */
			linked_list_file->previous = linked_list_file->current;	/*grab position of last node */
		}

		linked_list_file->iterator_state = active;	/*and set back to active */

		if ((linked_list_file->current = linked_list_file->next) == END_OF_LIST) {
			return err_item_not_found;
		}
		else {
			return fll_get(linked_list_file, ll_node);
		}
	}
}

err_t
fll_insert(
	ll_file_t		*linked_list_file,
	ll_file_node_t	*node
) {
	/* starting at head of list, read in and compare each node */
	ll_file_node_t *ll_probe;

	ll_probe = (ll_file_node_t *) malloc(linked_list_file->node_size);

	/*est iterator to the start of the list */
	/*@todo add check to improve performance as list may already be active for multiple inserts*/
	fll_reset(linked_list_file);

	fll_get(linked_list_file, ll_probe);/*Read in the head node and determine what to do*/

	/*Check empty list case and if true, just insert and assume it will always be at end of file*/
	/*@todo this needs improvement -> tail list */
	if (ll_probe->next == END_OF_LIST) {
		long int	ll_probe_pos	= linked_list_file->current;		/*Get position that will be written at */
		long int	pos				= fll_write_node(linked_list_file, node);	/*This will return the position in the file which will need to be updated in probe */

#if DEBUG
		DUMP(ll_probe_pos, "%li");	/*this is the position of the current read */
		DUMP(pos, "%li");
		DUMP(node->next, "%i");
		DUMP(*(int *) node->data, "%i");
#endif
		ll_probe->next				= pos;								/*update previous node's next */
		fll_update_node(linked_list_file, ll_probe, ll_probe_pos);
		linked_list_file->current	= ll_probe_pos + linked_list_file->node_size;
		/*Update current position in file as it is now just after updated node */
	}
	else {
		/* The first node is just the head that contains no data! Just a pointer to the first node */
		long int	ll_previous_node_pos;
		boolean_t	done = 0;

		do {
			/*move to the next node as the node to insert is larger */
			ll_previous_node_pos = linked_list_file->current;	/*Position of the previous node */

			/* if you've hit the end of the list, just insert */
			if (ll_probe->next == END_OF_LIST) {
				/*You've hit the end of the list, so just append */
				ll_previous_node_pos = linked_list_file->current;

				long int pos = fll_write_node(linked_list_file, node);	/*This will return the position in the file which will need to be updated in probe */

				ll_probe->next				= pos;
				fll_update_node(linked_list_file, ll_probe, ll_previous_node_pos);
				linked_list_file->current	= ll_previous_node_pos + linked_list_file->node_size;
				done						= 1;
			}
			else {
				fll_next(linked_list_file, ll_probe);
#if DEBUG
				DUMP(ll_previous_node_pos, "%li");
				DUMP(ll_probe->next, "%i");
				DUMP(*(int *) node->data, "%i");
				DUMP(*(int *) ll_probe->data, "%i");
				DUMP(linked_list_file->compare, "%p");
#endif

				if (linked_list_file->compare(linked_list_file, node, ll_probe) < 0) {
					/* HEATH //NEEDS TO BE INSPECTED FURTHER */
					/*this is where the node gets inserted*/
					node->next = linked_list_file->current;	/*update forward link */

					long int pos = fll_write_node(linked_list_file, node);	/*This will return the position in the file which will need to be updated in probe */

					/* read in previous node */
					linked_list_file->current	= ll_previous_node_pos;
					/*This should be cached? */
					fll_get(linked_list_file, ll_probe);
					ll_probe->next				= pos;
					fll_update_node(linked_list_file, ll_probe, ll_previous_node_pos);
					linked_list_file->current	= pos;
					done						= 1;
				}
			}
		} while (!done);
	}

	free(ll_probe);	/*clean up probe */
	return 0;	/*@todo Fix return codes */
}

err_t
fll_delete(
	ll_file_t	*linked_list_file,
	ion_key_t	key
) {
	/*@todo need state to prevent erronious deletes */
	/*@todo duplicate code with remove? */
	/*@todo This can be optimized so that the iterator does not need to be reset ?*/
	fll_reset(linked_list_file);

	ll_file_node_t *ll_node = (ll_file_node_t *) malloc(linked_list_file->node_size);

	fll_find(linked_list_file, key, ll_node);

	/* load the previous node */
	long int next		= ll_node->next;						/* this is the position that the previous node will point to */
	/* load previous node */
	long int current	= linked_list_file->current;			/* store position of current node */

	linked_list_file->current			= linked_list_file->previous;	/* load previous node */
	fll_get(linked_list_file, ll_node);
	ll_node->next						= next;					/* and point the previous node to the next */
	fll_update_node(linked_list_file, ll_node, linked_list_file->previous);
	/* store location */
	linked_list_file->current			= current;				/* correct pointer */
	free(ll_node);

	linked_list_file->iterator_state	= uninitialized;		/*and as decided, uninitialize the iterator */
	return 0;	/*@fix return codes*/
}

/*@todo Clean up memory management */
err_t
fll_find(
	ll_file_t		*linked_list_file,
	ion_key_t		key,
	ll_file_node_t	*ll_node
) {
	/* check the value of the current node */
	/* create a node for comparison */
	ll_file_node_t *ll_search_node = (ll_file_node_t *) malloc(linked_list_file->node_size);

	/*This will fall out of scope when function exits */

	memcpy(ll_search_node->data, key, sizeof(int));	/*todo key size needs to be resolved to make it consistent */

	if (linked_list_file->iterator_state == uninitialized) {
		/*If you are looking into the file, you will need iterator so init if needed */
		fll_reset(linked_list_file);
	}

	fll_get(linked_list_file, ll_node);

	int equal = 1;	/*Force inequality in the event of short circuit */

	if ((linked_list_file->current != HEAD_NODE) && ((equal = fll_compare(linked_list_file, ll_search_node, ll_node)) == 0)) {
		free(ll_search_node);
		return err_ok;
	}
	else if (equal < 0) {
		/*The node being searched for has not yet been found from current post																	so keep searching forward */
		fll_reset(linked_list_file);
	}

	while (fll_next(linked_list_file, ll_node) != err_item_not_found) {
		if (fll_compare(linked_list_file, ll_search_node, ll_node) == 0) {
			free(ll_search_node);
			return err_ok;
		}
	}

	free(ll_search_node);
	return err_item_not_found;
}

err_t
fll_get(
	ll_file_t				*linked_list_file,
	volatile ll_file_node_t *ll_node
) {
	if (0 == fseek(linked_list_file->file, linked_list_file->current, SEEK_SET)) {
		if (1 == fread((char *) ll_node, linked_list_file->node_size, 1, linked_list_file->file)) {
			linked_list_file->next = ll_node->next;	/*get next */
			return err_ok;
		}
	}

	return err_file_read_error;
}

err_t
fll_create_node(
	ll_file_t		*linked_list_file,
	record_info_t	*record,
	ion_key_t		key,
	ion_value_t		value,
	ll_file_node_t	**node
) {
	*node = (ll_file_node_t *) malloc(linked_list_file->node_size);	/*Allocate enough space for node */

	ll_file_node_t *ll_node = (ll_file_node_t *) (*node);

	memcpy(ll_node->data, key, record->key_size);
	memcpy(ll_node->data + record->key_size, value, record->value_size);
	ll_node->next = END_OF_LIST;

#if DEBUG
	DUMP(linked_list_file->node_size, "%i");
	DUMP(*(int *) node, "%i");
	DUMP(*(int *) (ll_node->data), "%i");
	DUMP(ll_node->next, "%i");
	io_printf("value  %.*s\n", record->value_size, (ll_node->data + record->key_size));
#endif

	/*@todo fix return code */
	return 0;
}

int
fll_compare(
	ll_file_t		*linked_list_file,
	ll_file_node_t	*base_node,
	ll_file_node_t	*compare_node
) {
	UNUSED(linked_list_file);
#if DEBUG
	DUMP(*(int *) (compare_node->data), "%i");
	DUMP(*(int *) (base_node->data), "%i");
#endif
	return *(int *) base_node->data - *(int *) (compare_node->data);
}

long int
fll_write_node(
	ll_file_t		*linked_list_file,
	ll_file_node_t	*head_node
) {
	/* write out node at end of file */
	fseek(linked_list_file->file, 0, SEEK_END);	/*move to the end of the file */

	long int cur_pos = ftell(linked_list_file->file);	/*record current position */

#if DEBUG && DEBUG_LEVEL > LOW
	DUMP(cur_pos, "%li");
	DUMP(linked_list_file->node_size, "%i");

	int i;

	for (i = 0; i < linked_list_file->node_size; i++) {
		DUMP(*((char *) (head_node) + i), "%x");
	}

#endif
	fwrite((char *) head_node, linked_list_file->node_size, 1, linked_list_file->file);
	/*write node */
	fflush(linked_list_file->file);	/*and flush it out */

	/*@todo update previous node position */
	linked_list_file->current = cur_pos + linked_list_file->node_size;
	/*index is now at end of file */
#if DEBUG

	long int act_pos = ftell(linked_list_file->file);

	DUMP(act_pos, "%li");
#endif
	return cur_pos;
}

err_t
fll_update_node(
	ll_file_t		*linked_list_file,
	ll_file_node_t	*ll_probe,
	long int		ll_probe_pos
) {
#if DEBUG
	DUMP(ll_probe_pos, "%li");
	DUMP(linked_list_file->node_size, "%i");
#endif
	fseek(linked_list_file->file, ll_probe_pos, SEEK_SET);	/*move to correct position in file */
	fwrite(ll_probe, linked_list_file->node_size, 1, linked_list_file->file);
	/*Update node -> write out entire node as we don't know what's been changed*/
	return 0;
}

err_t
fll_update(
	ll_file_t		*linked_list_file,
	ll_file_node_t	*ll_probe
) {
	return fll_update_node(linked_list_file, ll_probe, linked_list_file->current);
}

err_t
fll_remove(
	ll_file_t *linked_list_file
) {
	if (linked_list_file->current == END_OF_LIST) {
		/*if the iterator is at the end of the list, you can't remove anything */
		return err_illegal_state;
	}

	switch (linked_list_file->iterator_state) {
		case uninitialized:
			return err_uninitialized;

		case initialized:
			return err_illegal_state;	/*iterator is init, but next not called yet*/

		case removed:
			return err_illegal_state;

		default:/*it's okay to try to delete */
		{
			/* load the previous node */
			ll_file_node_t *ll_node = (ll_file_node_t *) malloc(linked_list_file->node_size);
			/* long int current					 = linked_list_file->current;/ ** store the current node * / */
			long int next			= linked_list_file->next;

			linked_list_file->current	= linked_list_file->previous;
			/*back up and get previous node */
			fll_get(linked_list_file, ll_node);
			ll_node->next				= next;								/*and point the previous node to the next */
			fll_update_node(linked_list_file, ll_node, linked_list_file->previous);
			/*This does not have to get updated ? */
			/* linked_list_file->current			= current;					/ ** return state * / */
			linked_list_file->next				= next;
			linked_list_file->iterator_state	= removed;					/*note that the item has been removed */
			free(ll_node);
			break;
		}
	}

	return err_ok;
}

err_t
fll_close(
	ll_file_t *linked_list_file
) {
	err_t err;

	if (linked_list_file->file != NULL) {
		err = fclose(linked_list_file->file);
		free(linked_list_file->file_name);

		if (err == 0) {
			return err_ok;
		}
		else {
			return err_file_write_error;
		}
	}

	return err_file_close_error;
}
