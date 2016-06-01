#include <stdio.h>
#include "./../kv_system.h"
#include "./../dictionary/dictionary.h"
#include "../dictionary/bpp_tree/bpp_tree_handler.h"
#include "../dictionary/ion_master_table.h"

int
main(
	void
)
{
	int                     key_size, value_size;
	key_type_t              key_type;
	dictionary_handler_t    bpp_tree_handler;
	dictionary_t            dictionary;
	status_t                status;

	key_size 	= 4;
	key_type 	= key_type_numeric_signed;
	value_size 	= 14;

	int	            key 		= 1;
	unsigned char   value[14] 	= "Hello World 0";

	printf("Creating the master table...\n");
	if (ion_init_master_table() != err_ok) {
		printf("Failed to create the master table\n");
		return 1;
	}

	bpptree_init(&bpp_tree_handler);

	printf("Create the dictionary using a BPP Tree\n");
	if (ion_master_table_create_dictionary(&bpp_tree_handler, &dictionary, key_type, key_size, value_size, -1) != err_ok) {
		printf("Failed to create the dictionary\n");
		return 1;
	}

	printf("Inserting (%d|%s)...\n", key, value);
	status = dictionary_insert(&dictionary, IONIZE(key), value);
	if (status != err_ok) {
		printf("Inserting (%d|%s) failed\n", key, value);
		return 1;
	}

	printf("Deleting (%d)...\n", key);
	status = dictionary_delete(&dictionary, IONIZE(key));
	if (status != err_ok) {
		printf("Deleting (%d) failed\n", key);
		return 1;
	}

	printf("Updating (%d with %s)...\n", key, value);
	status = dictionary_update(&dictionary, IONIZE(key), value);
	if (status != err_ok) {
		printf("Updating (%d with %s) failed\n", key, value);
		return 1;
	}

	int i;
	for (i = 1; i < 3; i++) {
		printf("Inserting (%d|%s)...\n", key, value);
		sprintf((char *) value, "Hello World %d", i);
		status = dictionary_insert(&dictionary, IONIZE(key), value);

		if (status != err_ok) {
			printf("Inserting (%d|%s) failed\n", key, value);
			return 1;
		}
	}

	printf("Querying (%d)...\n", key);
	unsigned char new_value[value_size];
	status = dictionary_get(&dictionary, IONIZE(key), new_value);
	if (status == err_ok) {
		printf("Got the value back of '%s' stored in %d.\n", value, key);
	}
	else {
		printf("Failed to query (%d)\n", key);
		return 1;
	}

	printf("Iterating through all of the records...\n");
	predicate_t predicate;
	if (dictionary_build_predicate(&predicate, predicate_equality, IONIZE(key)) != err_ok) {
		printf("Failed to build predicate\n");
		return 1;
	}

	dict_cursor_t *cursor = NULL;
	if (dictionary_find(&dictionary, &predicate, &cursor) != err_ok) {
		printf("Failed to set up cursor\n");
		return 1;
	}

	ion_record_t ion_record;
	ion_record.key      = malloc((size_t) key_size);
	ion_record.value    = malloc((size_t) value_size);

	cursor_status_t cursor_status;
	while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active ||
			cursor_status == cs_cursor_initialized) {
		printf("\tKey: %d, Value: %s\n", NEUTRALIZE(int, ion_record.key), (char *) ion_record.value);
	}

	if (cursor_status != cs_end_of_results) {
		printf("Failed to read all of the records");
		return 1;
	}

	cursor->destroy(&cursor);
	free(ion_record.key);
	free(ion_record.value);

	printf("Deleting dictionary...\n");
	if (ion_delete_from_master_table(&dictionary)) {
		printf("Failed to delete the dictionary\n");
		return 1;
	}

	printf("Closing and deleting the master table...\n");
	if (ion_close_master_table() != err_ok || ion_delete_master_table() != err_ok) {
		printf("Failed to close and delete the master table\n");
		return 1;
	}

	printf("Done\n");
	return 0;
}
