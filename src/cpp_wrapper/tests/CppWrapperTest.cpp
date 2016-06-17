//
// Created by Dana Klamut on 2016-05-31.
//

#include <iostream>

#include "../../dictionary/ion_master_table.h"
#include "../MasterTable.h"

using namespace std;

int main() {
	/** The following is example test code for using Dictionary.h */

//    Dictionary<int> *btree = new BppTree<int>(key_type_numeric_signed, sizeof(int), 10);
//
//	printf("Insert %d [%s]\n", 3, "hello!");
//	string s = "hello!";
//	btree->insert(3, s);
//
//	string buf = btree->get<string>(3);
//	cout << "\nRetrieve key 3 and got back " << buf << endl;
//
//	cout << "\nUpdating value of key 3 to hi!" << endl;
//
//	string t = "hi!";
//	btree->update(3, t);
//
//	string buf2 = btree->get<string>(3);
//	cout << "\nRetrieve key 3 after update and got back " << buf2 << endl;
//
//	cout << "\nDelete key 3" << endl;
//	btree->deleteRecord(3);
//
//	cout << "\nTry and retrieve key 3 after deletion" << endl;
//
//	string buf3 = btree->get<string>(3);
//
//	cout << "\nRetrieve key 3 and got back " << buf3 << endl;
//
//	printf("\nInsert %d [%s]\n", 3, "test1!");
//	string u = "test1!";
//	btree->insert(3, u);
//
//	printf("Insert %d [%s]\n", 3, "test2!");
//	string v = "test2!";
//	btree->insert(3, v);
//
//	printf("Insert %d [%s]\n", 4, "test3!");
//	string w = "test3!";
//	btree->insert(4, w);
//
//	cout << "\nTesting equality query on key 3: " << endl;
//	dict_cursor_t *cursor1 = btree->equality(3);
//	ion_record_t rec11 = btree->getRecord(cursor1);
//	int key11 = *((int*)rec11.key);
//	cout << "Key: " << key11 << " Value: " << rec11.value << endl;
//	ion_record_t rec12 = btree->getRecord(cursor1);
//	int key12 = *((int*)rec12.key);
//	cout << "Key: " << key12 << " Value: " << rec12.value << endl;
//	ion_record_t rec13 = btree->getRecord(cursor1);
//	int key13 = *((int*)rec13.key);
//	cout << "Key: " << key13 << " Value: " << rec13.value << endl;
//
//	cout << "\nTesting all records query: " << endl;
//	dict_cursor_t *cursor2 = btree->allRecords();
//	ion_record_t rec21 = btree->getRecord(cursor2);
//	int key21 = *((int*)rec21.key);
//	cout << "Key: " << key21 << " Value: " << rec21.value << endl;
//	ion_record_t rec22 = btree->getRecord(cursor2);
//	int key22 = *((int*)rec22.key);
//	cout << "Key: " << key22 << " Value: " << rec22.value << endl;
//	ion_record_t rec23 = btree->getRecord(cursor2);
//	int key23 = *((int*)rec23.key);
//	cout << "Key: " << key23 << " Value: " << rec23.value << endl;
//	ion_record_t rec24 = btree->getRecord(cursor2);
//	int key24 = *((int*)rec24.key);
//	cout << "Key: " << key24 << " Value: " << rec24.value << endl;
//
//	cout << "\nTesting range query: 2<=key<=3 " << endl;
//	dict_cursor_t *cursor3 = btree->range(2, 3);
//	ion_record_t rec31 = btree->getRecord(cursor3);
//	int key31 = *((int*)rec31.key);
//	cout << "Key: " << key31 << " Value: " << rec31.value << endl;
//	ion_record_t rec32 = btree->getRecord(cursor3);
//	int key32 = *((int*)rec32.key);
//	cout << "Key: " << key32 << " Value: " << rec32.value << endl;
//	ion_record_t rec33 = btree->getRecord(cursor3);
//	int key33 = *((int*)rec33.key);
//	cout << "Key: " << key33 << " Value: " << rec33.value << endl;
//
//	/** Master table method calls need more thought */
//	//printf("Creating the master table...\n");
//	//btree->createMasterTable();
//
//	cout << "\nEnd cpp wrapper test." << endl;
//
//	btree->destroy();
//	delete btree;

	/** The following is example test code for using MasterTable.h */

	int						key_size, value_size;
	key_type_t				key_type;
	dictionary_handler_t	bpp_tree_handler;
	dictionary_t			dictionary;

	/* These sizes are given in units of bytes. */
	key_type	= key_type_numeric_signed;
	key_size	= 4;
	value_size	= 14;

	/* Here, we make the assumption that the value is going to be a null-terminated string. */
	int				key = 1;
	unsigned char	value[value_size];

	sprintf((char *) value, "Hello World 0");

	/*
		Initializing the master table creates the master table record, which is a file written to disk.
		Note that we wrap each function call in an if statement to check its return code. They don't have to be done
		"In-line" like what's done here.
	*/
	printf("Creating the master table...\n");

	/** Yields undefined symbols error as it references ion_init_master_table() */
	MasterTable *test = new MasterTable();
	printf("Master table created \n");

	/* This writes to the master table record, and initializes the dictionary. */
	printf("Create the dictionary using a BPP Tree\n");

	if (test->createBppTreeDictionary(key_type, key_size, value_size) != err_ok) {
		printf("Failed to create the dictionary\n");
		return 1;
	}

	/*
		Insertions take a key and a value that have been properly ionized. "Ionization" is the process of converting
		a key or value from its normal state, into a "byte array". The layout of the data is still exactly the same,
		but ionization erases type information to allow IonDB to remain type-agnostic.
	*/
//	printf("Inserting (%d|%s)...\n", key, value);
//	test->
//	status = dictionary_insert(&dictionary, IONIZE(key, int), value);
//
//	if (status != err_ok) {
//		printf("Inserting (%d|%s) failed\n", key, value);
//		return 1;
//	}
//
//	/* Deletion removes all records stored with the given key (Since duplicate keys are allowed). */
//	printf("Deleting (%d)...\n", key);
//	status = dictionary_delete(&dictionary, IONIZE(key, int));
//
//	if (status != err_ok) {
//		printf("Deleting (%d) failed\n", key);
//		return 1;
//	}
//
//	/*
//		Updates take a new value, and replaces all values stored at the given key, similar to how delete works.
//		If the key doesn't currently exist, it will be inserted instead of being updated.
//	*/
//	unsigned char replace_value[value_size];
//
//	sprintf((char *) replace_value, "Hello new!");
//	printf("Updating (%d with %s)...\n", key, replace_value);
//	status = dictionary_update(&dictionary, IONIZE(key, int), replace_value);
//
//	if (status != err_ok) {
//		printf("Updating (%d with %s) failed\n", key, replace_value);
//		return 1;
//	}
//
//	/* 4 Records are inserted to demonstrate the cursor queries. */
//	int i;
//
//	for (i = 2; i < 6; i++) {
//		sprintf((char *) value, "Hello World %d", i);
//		printf("Inserting (%d|%s)...\n", key, value);
//		status = dictionary_insert(&dictionary, IONIZE(key, int), value);
//
//		if (status != err_ok) {
//			printf("Inserting (%d|%s) failed\n", key, value);
//			return 1;
//		}
//	}
//
//	/* A get operation requires the user to allocate space to store the returned value. Only retrieves one record. */
//	printf("Querying (%d)...\n", key);
//
//	unsigned char new_value[value_size];
//
//	status = dictionary_get(&dictionary, IONIZE(key, int), new_value);
//
//	if (status == err_ok) {
//		printf("Got the value back of '%s' stored in %d.\n", value, key);
//	}
//	else {
//		printf("Failed to query (%d)\n", key);
//		return 1;
//	}
//
//	/*
//		First step to a cursor query is to initialize the predicate. How this is done depends on the type of query.
//		For the equality query, we're asking for all records with an exact match to the given key to be returned.
//	*/
//	printf("Iterating through all records with key = %d...\n", key);
//
//	predicate_t predicate;
//
//	if (dictionary_build_predicate(&predicate, predicate_equality, IONIZE(key, int)) != err_ok) {
//		printf("Failed to build predicate\n");
//		return 1;
//	}
//
//	/*
//		Once we have the predicate, now we call the find operation to set-up the cursor. Note that we pass in the cursor
//		uninitialized on purpose, since find will handle the construction of the cursor.
//	*/
//	dict_cursor_t *cursor = NULL;
//
//	if (dictionary_find(&dictionary, &predicate, &cursor) != err_ok) {
//		printf("Failed to set up cursor\n");
//		return 1;
//	}
//
//	/* Allocate space in preparation for retrieving records. */
//	ion_record_t ion_record;
//
//	ion_record.key		= malloc((size_t) key_size);
//	ion_record.value	= malloc((size_t) value_size);
//
//	/*
//		Start iteration through the cursor. We keep iterating as long as the cursor is either initialized
//		("one-before" the first record) or active (in the process of iteration). Each time we call "next" on the cursor,
//		A new record is returned by writing into the given ion_record.
//	*/
//	cursor_status_t cursor_status;
//
//	while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
//		printf("\tKey: %d, Value: %s\n", NEUTRALIZE(ion_record.key, int), (char *) ion_record.value);
//	}
//
//	/* Check the end result, since the cursor may have terminated due to error instead finishing properly. */
//	if (cursor_status != cs_end_of_results) {
//		printf("Failed to read all of the records");
//		return 1;
//	}
//
//	/* Clean-up everything by removing the cursor. Also free the ion_record space we allocated. */
//	cursor->destroy(&cursor);
//	free(ion_record.key);
//	free(ion_record.value);

	/* Clean-up the dictionary. Must be done through the master table to erase the table record we kept. */
	printf("Deleting dictionary...\n");

	if (err_ok != test->deleteFromMasterTable()) {
		printf("Failed to delete the dictionary\n");
		return 1;
	}

	/* Close, and then remove the master table. This deletion will erase the master record from disk. */
	printf("Closing and deleting the master table...\n");

	if ((test->closeMasterTable() != err_ok) || (test->deleteMasterTable() != err_ok)) {
		printf("Failed to close and delete the master table\n");
		return 1;
	}

	printf("Done\n");
	return 0;
}