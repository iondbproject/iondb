/******************************************************************************/
/**
@file		skiplist.c
@author		Kris Wallperington
@brief		Functional test for Skiplist KV implementation
*/
/******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "./../kv_system.h"
#include "./../dictionary/dictionary.h"
#include "./../dictionary/skiplist/slhandler.h"
#include "./../kv_io.h"

int
main(
	void
)
{
	io_printf("%s\n", "I'm a skip list!");
	io_printf("%s\n\n", "Commands:\nInsert (I <key> <value> | 'I 23 test') " \
			"Delete (D <key> | 'D 64') Update (U <key> <new value> | 'U 99 " \
			"muffin')\nQuery (Q <key> | 'Q 16') Print (P | 'P') Find Node (" \
			"F <key> | 'F 72')\n\nX to stop.");

	int key_size, value_size, maxheight;
	key_type_t key_type;
	dictionary_handler_t skip_handler;
	dictionary_t test_dict;

	key_size 	= 4;
	key_type 	= key_type_numeric_signed;
	value_size 	= 10;
	maxheight 	= 7;

	sldict_init(&skip_handler);

	dictionary_create(&skip_handler, &test_dict, 1, key_type, key_size,
														value_size, maxheight);

	char in;
	while( (in = getchar()) != 'X')
	{
#if DEBUG
		io_printf("You said: %c\n", in);
#endif

		switch(in)
		{
			case 'P': {
#if DEBUG
				io_printf("%s\n", "Printing skiplist");
#endif
				print_skiplist((skiplist_t*) test_dict.instance);
				break;
			}

			case 'I': {
							int		 key;
				unsigned 	char	 value[10];
				scanf("%i %s", &key, value);
#if DEBUG
				io_printf("Inserting (%d|%s)...\n", key, value);
#endif
				dictionary_insert(&test_dict, (ion_key_t) &key, value);
				break;
			}

			case 'D': {
				int key;
				scanf("%i", &key);
#if DEBUG
				io_printf("Deleting (%d)...\n", key);
#endif
				dictionary_delete(&test_dict, (ion_key_t) &key);
				break;
			}

			case 'U': {
							int 	key;
				unsigned 	char 	value[10];
				scanf("%i %s", &key, value);
#if DEBUG
				io_printf("Updating (%d with %s)...\n", key, value);
#endif
				dictionary_update(&test_dict, (ion_key_t) &key, value);
				break;
			}

			case 'Q': {
							int 	key;
				unsigned 	char 	value[10];
				scanf("%i", &key);
#if DEBUG
				io_printf("Querying (%d)...\n", key);
#endif
				dictionary_get(&test_dict, (ion_key_t) &key, value);
				io_printf("Got the value back of '%s' stored in %d.\n", value,
																		key);
				break;
			}
			case 'F': {
				int key;
				scanf("%i", &key);
#if DEBUG
				io_printf("Finding Node (%d)...\n", key);
#endif
				sl_node_t 	*node = sl_find_node(
										(skiplist_t*) test_dict.instance,
										(ion_key_t) &key
									);
				io_printf("Got back node, key '%d'.\n",
							NULL != node->key ? *((int*) node->key) : -1337);
				break;
			}
		}

		io_printf("%s", "\n");
		getchar(); /* Eat newline */
	}

	dictionary_delete_dictionary(&test_dict);
	return 0;
}
