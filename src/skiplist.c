/******************************************************************************/
/**
@file		skiplist.c
@author		Kris Wallperington
@brief		Functional test for Skiplist KV implementation
*/
/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "kv_system.h"
#include "./dictionary/dictionary.h"
#include "./dictionary/skiplist/slhandler.h"
#include "io.h"

/* Copied from top of test slstore.c and renamed */
void
print_skiplist(
	skiplist_t 	*skiplist
)
{
	sl_node_t 	*cursor = skiplist->head;

	while(NULL != cursor->next[0])
	{
		int 		key 		= *((int*)cursor->next[0]->key);
		char* 		value 		= (char*) cursor->next[0]->value;
		sl_level_t 	level 		= cursor->next[0]->height + 1;
		io_printf("k: %d (v: %s) [l: %d] -- ", key, value, level);
		cursor = cursor->next[0];
	}

	io_printf("%s", "END\n\n");
}

int
main(
	void
)
{
	io_printf("%s\n", "I'm a skip list!");
	io_printf("%s\n\n", "Commands:\nInsert (I <key> <value> | 'I 23 test') " \
			"Delete (D <key> | 'D 64') Update (U <key> <new value> | 'U 99 " \
			"muffin')\nQuery (Q <key> | 'Q 16') Print (P | 'P')\n\nX to stop.");

	int key_size, value_size, maxheight;
	key_type_t key_type;
	key_size 	= 4;
	key_type 	= key_type_numeric;
	value_size 	= 10;
	maxheight 	= 7;

	dictionary_handler_t skip_handler;

	sldict_init(&skip_handler);

	dictionary_t test_dict;

	dictionary_create(&skip_handler, &test_dict, key_type, key_size, value_size, maxheight);

	char in;
	while( (in = getchar()) != 'X')
	{
#ifdef DEBUG
		io_printf("You said: %c\n", in);
#endif

		switch(in)
		{
			case 'P':
			{
#ifdef DEBUG
				io_printf("%s\n", "Printing skiplist");
#endif
				print_skiplist((skiplist_t*) test_dict.instance);
				break;
			}

			case 'I':
			{
				int key;
				unsigned char value[10];
				scanf("%i %s", &key, value);
#ifdef DEBUG
				io_printf("Inserting (%d|%s)...\n", key, value);
#endif
				dictionary_insert(&test_dict, (ion_key_t) &key, value);
				break;
			}

			case 'D':
			{
				int key;
				scanf("%i", &key);
#ifdef DEBUG
				io_printf("Deleting (%d)...\n", key);
#endif
				dictionary_delete(&test_dict, (ion_key_t) &key);
				break;
			}

			case 'U':
			{
				int key;
				unsigned char value[10];
				scanf("%i %s", &key, value);
#ifdef DEBUG
				io_printf("Updating (%d with %s)...\n", key, value);
#endif
				dictionary_update(&test_dict, (ion_key_t) &key, value);
				break;
			}

			case 'Q':
			{
				int key;
				ion_value_t value;
				scanf("%i", &key);
#ifdef DEBUG
				io_printf("Querying (%d)...\n", key);
#endif
				dictionary_get(&test_dict, (ion_key_t) &key, &value);
				io_printf("Got the value back of '%s' stored in %d.\n", value, key);
				free(value);
				break;
			}
		}

		io_printf("%s", "\n");
		getchar(); // Eat newline
	}

	dictionary_delete_dictionary(&test_dict);
	return EXIT_SUCCESS;
}
