#include "linear_hash_handler.h"
#include "linear_hash.c"
#include "./../dictionary.h"
#include <stdio.h>
#include <stdlib.h>

void
clean_data_file(
) {
	FILE *delete_file;

	delete_file = fopen("data.bin", "w");
	fclose(delete_file);
}

int
main(
) {
	/* Clean data file */
	clean_data_file();

	ion_dictionary_t			dict;
	ion_dictionary_handler_t	handler;

	linear_hash_dict_init(&handler);
	dictionary_create(&handler, &dict, 1, key_type_numeric_unsigned, 1, 1, 1);
	dict.handler->insert(&dict, 1, 1);
	print_linear_hash_state(dict.instance);

	return 0;
}
