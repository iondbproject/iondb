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

int main(void) {
	io_printf("%s\n", "I'm a skip list!");
	ion_printf("%s\n", "Commands: Insert (I <key> <value> | 'I 23 test') " \
			"Delete (D <key> | 'D 64') Update (U <key> <new value> | 'U 99" \
			"muffin') Query (Q <key> | 'Q 16')");

	int key_size, value_size, maxheight;
	key_size 	= 4;
	value_size 	= 10;
	maxheight 	= 7;

	dictionary_handler_t skip_handler;

	sldict_init(&skip_handler);

	dictionary_t test_dictionary;

	dictionary_create(&skip_handler, &test_dictionary, key_size,
														value_size, maxheight);

	char in;
	while( (in = getchar()) != 'X')
	{
		getchar(); // Eat newline
		io_printf("You said: %c\n", in);
	}

	dictionary_delete_dictionary(&test_dictionary);
	return EXIT_SUCCESS;
}
