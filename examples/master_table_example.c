/******************************************************************************/
/**
@file		master_table_example.c
@author		IonDB Project
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
@par
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#include <stdio.h>
#include "../src/key_value/kv_system.h"
#include "../src/dictionary/dictionary.h"
#include "../src/dictionary/bpp_tree/bpp_tree_handler.h"
#include "../src/dictionary/ion_master_table.h"

int
main(
	void
) {
	int							key_size, value_size;
	ion_key_type_t				key_type;
	ion_dictionary_handler_t	bpp_tree_handler;
	ion_dictionary_t			dictionary;
	ion_status_t				status;

	/* These sizes are given in units of bytes. */
	key_type	= key_type_numeric_signed;
	key_size	= 4;
	value_size	= 14;

	/* Here, we make the assumption that the value is going to be a null-terminated string. */
	int			key = 1;
	ion_byte_t	value[value_size];

	sprintf((char *) value, "Hello World 0");

	/*
		Initializing the master table creates the master table record, which is a file written to disk.
		Note that we wrap each function call in an if statement to check its return code. They don't have to be done
		"In-line" like what's done here.
	*/
	printf("Creating the master table...\n");

	if (ion_init_master_table() != err_ok) {
		printf("Failed to create the master table\n");
		return 1;
	}

	/* Set-up the handler by registering it with the B+ Tree function pointers. */
	bpptree_init(&bpp_tree_handler);

	/* This writes to the master table record, and initializes the dictionary. */
	printf("Create the dictionary using a BPP Tree\n");

	if (ion_master_table_create_dictionary(&bpp_tree_handler, &dictionary, key_type, key_size, value_size, -1) != err_ok) {
		printf("Failed to create the dictionary\n");
		return 1;
	}

	/*
		Insertions take a key and a value that have been properly ionized. "Ionization" is the process of converting
		a key or value from its normal state, into a "byte array". The layout of the data is still exactly the same,
		but ionization erases type information to allow IonDB to remain type-agnostic.
	*/
	printf("Inserting (%d|%s)...\n", key, value);
	status = dictionary_insert(&dictionary, IONIZE(key, int), value);

	if (status.error != err_ok) {
		printf("Inserting (%d|%s) failed\n", key, value);
		return 1;
	}

	/* Deletion removes all records stored with the given key (Since duplicate keys are allowed). */
	printf("Deleting (%d)...\n", key);
	status = dictionary_delete(&dictionary, IONIZE(key, int));

	if (status.error != err_ok) {
		printf("Deleting (%d) failed\n", key);
		return 1;
	}

	/*
		Updates take a new value, and replaces all values stored at the given key, similar to how delete works.
		If the key doesn't currently exist, it will be inserted instead of being updated.
	*/
	ion_byte_t replace_value[value_size];

	sprintf((char *) replace_value, "Hello new!");
	printf("Updating (%d with %s)...\n", key, replace_value);
	status = dictionary_update(&dictionary, IONIZE(key, int), replace_value);

	if (status.error != err_ok) {
		printf("Updating (%d with %s) failed\n", key, replace_value);
		return 1;
	}

	/* 4 Records are inserted to demonstrate the cursor queries. */
	int i;

	for (i = 2; i < 6; i++) {
		sprintf((char *) value, "Hello World %d", i);
		printf("Inserting (%d|%s)...\n", key, value);
		status = dictionary_insert(&dictionary, IONIZE(key, int), value);

		if (status.error != err_ok) {
			printf("Inserting (%d|%s) failed\n", key, value);
			return 1;
		}
	}

	/* A get operation requires the user to allocate space to store the returned value. Only retrieves one record. */
	printf("Querying (%d)...\n", key);

	ion_byte_t new_value[value_size];

	status = dictionary_get(&dictionary, IONIZE(key, int), new_value);

	if (status.error == err_ok) {
		printf("Got the value back of '%s' stored in %d.\n", value, key);
	}
	else {
		printf("Failed to query (%d)\n", key);
		return 1;
	}

	/*
		First step to a cursor query is to initialize the predicate. How this is done depends on the type of query.
		For the equality query, we're asking for all records with an exact match to the given key to be returned.
	*/
	printf("Iterating through all records with key = %d...\n", key);

	ion_predicate_t predicate;

	if (dictionary_build_predicate(&predicate, predicate_equality, IONIZE(key, int)) != err_ok) {
		printf("Failed to build predicate\n");
		return 1;
	}

	/*
		Once we have the predicate, now we call the find operation to set-up the cursor. Note that we pass in the cursor
		uninitialized on purpose, since find will handle the construction of the cursor.
	*/
	ion_dict_cursor_t *cursor = NULL;

	if (dictionary_find(&dictionary, &predicate, &cursor) != err_ok) {
		printf("Failed to set up cursor\n");
		return 1;
	}

	/* Allocate space in preparation for retrieving records. */
	ion_record_t ion_record;

	ion_record.key		= malloc((size_t) key_size);
	ion_record.value	= malloc((size_t) value_size);

	/*
		Start iteration through the cursor. We keep iterating as long as the cursor is either initialized
		("one-before" the first record) or active (in the process of iteration). Each time we call "next" on the cursor,
		A new record is returned by writing into the given ion_record.
	*/
	ion_cursor_status_t cursor_status;

	while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
		printf("\tKey: %d, Value: %s\n", NEUTRALIZE(ion_record.key, int), (char *) ion_record.value);
	}

	/* Check the end result, since the cursor may have terminated due to error instead finishing properly. */
	if (cursor_status != cs_end_of_results) {
		printf("Failed to read all of the records");
		return 1;
	}

	/* Clean-up everything by removing the cursor. Also free the ion_record space we allocated. */
	cursor->destroy(&cursor);
	free(ion_record.key);
	free(ion_record.value);

	/* Clean-up the dictionary. Must be done through the master table to erase the table record we kept. */
	printf("Deleting dictionary...\n");

	if (ion_delete_dictionary(&dictionary, dictionary.instance->id)) {
		printf("Failed to delete the dictionary\n");
		return 1;
	}

	/* Close, and then remove the master table. This deletion will erase the master record from disk. */
	printf("Closing and deleting the master table...\n");

	if ((ion_close_master_table() != err_ok) || (ion_delete_master_table() != err_ok)) {
		printf("Failed to close and delete the master table\n");
		return 1;
	}

	printf("Done\n");
	return 0;
}
