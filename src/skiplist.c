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
	printf("%s\n", "I'm a skip list!");

	int key_size, value_size, maxheight;
	key_size 	= 4;
	value_size 	= 10;
	maxheight 	= 7;

	dictionary_handler_t skip_handler;

	sldict_init(&skip_handler);

	dictionary_t test_dictionary;

	dictionary_create(&skip_handler, &test_dictionary, key_size, value_size, maxheight);



	dictionary_delete_dictionary(&test_dictionary);
	return EXIT_SUCCESS;
}
