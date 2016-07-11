/*
 ============================================================================
 Name		: KV.c
 Author	  :
 Version	 :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdlib.h>
#include "../key_value/kv_system.h"
#include "./../dictionary/dictionary.h"
#include "../dictionary/open_address_hash/open_address_hash_dictionary_handler.h"

#include "../key_value/kv_io.h"

void
test_query(
	dictionary_t		*test_dictionary,
	int					key,
	const record_info_t *record
) {
	io_printf("Query\n");

	ion_value_t data;

	data = (ion_value_t) malloc(record->value_size);

	ion_status_t status = dictionary_get(&*test_dictionary, (ion_key_t) &key, (ion_value_t) data);

	if (status.error == err_ok) {
		int		idx;
		int		key = *((int *) data);
		char	*value;

		value = (char *) data + record->key_size;
		io_printf("Key: %i\n", key);

		for (idx = 0; idx < record->value_size; idx++) {
			io_printf("%c", value[idx]);
		}

		io_printf("\n");
		free(data);
	}
	else {
		io_printf("error %i\n", status.error);
	}
}

int
main(
	void
) {
	int				size;
	record_info_t	record;

	/* this is required for initializing the hash map and should come from the dictionary */
	record.key_size		= 4;
	record.value_size	= 10;
	size				= 10;

	dictionary_handler_t map_handler;	/* create handler for hashmap */

	/* TODO streamline process for */
	oadict_init(&map_handler);	/* register handler for hashmap */

	/* collection handler for test collection */
	dictionary_t test_dictionary;

	/* register the appropriate handler for a given collection */
	dictionary_create(&map_handler, &test_dictionary, 1, key_type_numeric_signed, record.key_size, record.value_size, size);

	io_printf("Insert\n");

	char	value[record.value_size];
	int		key = 6;	/* key */

	memcpy(value, "this is a ", record.value_size);

	dictionary_insert(&test_dictionary, (ion_key_t) &key, (ion_value_t) value);

	test_query(&test_dictionary, key, &record);

	memcpy(value, "this is b ", record.value_size);
	dictionary_update(&test_dictionary, (ion_key_t) &key, (ion_value_t) value);

	test_query(&test_dictionary, key, &record);

	return 0;
}
