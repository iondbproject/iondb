/******************************************************************************/
/**
@file		CppWrapperTest.cpp
@author		Dana Klamut
@brief		Test class exemplifying uses for Dictionary.h.
*/
/******************************************************************************/

#include <iostream>

using namespace std;

#include "../Dictionary.h"
#include "../BppTree.h"

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
//	cout << "\nEnd cpp wrapper test." << endl;
//
//	btree->destroy();
//	delete btree;

	Dictionary<string> *btree = new BppTree<string>(key_type_char_array, 10, sizeof(int));

	/* Insert into bpp tree */
	cout << ("Insert key_1 [1]") << endl;
	string key_1 = "key_1";
	int value = 1;
	btree->insert(key_1, value);

	/* Return the value inserted into the bpp tree */
	int returned_value = btree->get<int>(key_1);
	cout << "Retrieve key key_1 and got back: " << endl;
	cout << returned_value << endl;

	cout << "Updating value of key key_1 to 2" << endl;

	/* Update value */
	int new_value = 2;
	btree->update(key_1, new_value);

	/* Verify update was successful */
	returned_value = btree->get<int>(key_1);
	cout << "Retrieve key key_1 after update and got back: " << endl;
	cout << returned_value << endl;

	/* Delete key and value previously inserted */
	cout << "Delete key key_1" << endl;
	btree->deleteRecord(key_1);

	/* Verify delete was successful */
	cout << "Try and retrieve key key_1 after deletion" << endl;
	returned_value = btree->get<int>(key_1);
	cout << "Retrieve key key_1 and got back " << endl;
	cout << returned_value << endl;
	/* Returned value was nonsense characters, therefore we know the deletion was successful */

	/* Insert 3 key-value pairs to later test cursor queries */
	cout << "Insert key_1 [100]" << endl;
	int value_1 = 100;
	btree->insert(key_1, value_1);

	cout << "Insert key_1 [200]" << endl;
	int value_2 = 200;
	btree->insert(key_1, value_2);

	cout << "Insert key_2 [300]" << endl;
	string key_2 = "key_2";
	int value_3 = 300;
	btree->insert(key_2, value_3);

	/* Perform equality query on the key of 3 */
	cout << "Testing equality query on key key_1: " << endl;
	dict_cursor_t *cursor = btree->equality(key_1);
	ion_record_t record = btree->getRecord(cursor);
	string returned_key = *((string*)record.key);
	int cursor_returned_value = *((int*)record.value);

	/* First record returned */
	cout << "Key: " << endl;
	cout << returned_key << endl;
	cout << " Value: " << endl;
	cout << cursor_returned_value << endl;

	/* Second record returned */
	record = btree->getRecord(cursor);
	returned_key = *((string*)record.key);
	int cursor_returned_value2 = *((int*)record.value);
	cout << "Key: " << endl;
	cout << returned_key << endl;
	cout << " Value: " << endl;
	cout << cursor_returned_value2 << endl;

	/* Third record returned */
	record = btree->getRecord(cursor);
	returned_key = *((string*)record.key);
	int cursor_returned_value3 = *((int*)record.value);
	cout << "Key: " << endl;
	cout << returned_key << endl;
	cout << " Value: " << endl;
	cout << cursor_returned_value3 << endl;
	/* No value is returned, therefore we know there are no further records to search for */

	/* Perform an all records query */
	cout << "Testing all records query: " << endl;
	cursor = btree->allRecords();
	record = btree->getRecord(cursor);
	returned_key = *((string*)record.key);
	int cursor_returned_value4 = *((int*)record.value);

	/* First record returned */
	cout << "Key: " << endl;
	cout << returned_key << endl;
	cout << " Value: " << endl;
	cout << cursor_returned_value4 << endl;

	/* Second record returned */
	record = btree->getRecord(cursor);
	returned_key = *((string*)record.key);
	int cursor_returned_value5 = *((int*)record.value);
	cout << "Key: " << endl;
	cout << returned_key << endl;
	cout << " Value: " << endl;
	cout << cursor_returned_value5 << endl;

	/* Third record returned */
	record = btree->getRecord(cursor);
	returned_key = *((string*)record.key);
	int cursor_returned_value6 = *((int*)record.value);
	cout << "Key: " << endl;
	cout << returned_key << endl;
	cout << " Value: " << endl;
	cout << cursor_returned_value6 << endl;

	/* Fourth record returned */
	record = btree->getRecord(cursor);
	returned_key = *((string*)record.key);
	int cursor_returned_value7 = *((int*)record.value);
	cout << "Key: " << endl;
	cout << returned_key << endl;
	cout << " Value: " << endl;
	cout << cursor_returned_value7 << endl;
	/* No value is returned, therefore we know there are no further records to search for */

	/* Perform range query on keys k for "key_0"<=k<="key_1" */
	cout << "Testing range query: \"key_0\"<=key<=\"key_1\" " << endl;
	cursor = btree->range("key_0", "key_1");
	record = btree->getRecord(cursor);
	returned_key = *((string*)record.key);
	int cursor_returned_value8 = *((int*)record.value);

	/* First record returned */
	cout << "Key: " << endl;
	cout << returned_key << endl;
	cout << " Value: " << endl;
	cout << cursor_returned_value8 << endl;

	/* Second record returned */
	record = btree->getRecord(cursor);
	returned_key = *((string*)record.key);
	int cursor_returned_value9 = *((int*)record.value);
	cout << "Key: " << endl;
	cout << returned_key << endl;
	cout << " Value: " << endl;
	cout << cursor_returned_value9 << endl;

	/* Third record returned */
	record = btree->getRecord(cursor);
	returned_key = *((string*)record.key);
	int cursor_returned_value10 = *((int*)record.value);
	cout << "Key: " << endl;
	cout << returned_key << endl;
	cout << " Value: " << endl;
	cout << cursor_returned_value10 << endl;
	/* No value is returned, therefore we know there are no further records to search for */

	/* All methods have been tested */
	cout << "End cpp wrapper test." << endl;

	btree->destroy();
	delete btree;
}