/******************************************************************************/
/**
@file
@author		Dana Klamut
@brief		Test class exemplifying uses for Dictionary.h.
*/
/******************************************************************************/

#include <iostream>

using namespace std;

#include "../../../cpp_wrapper/Dictionary.h"
#include "../../../cpp_wrapper/BppTree.h"
#include "../../../cpp_wrapper/FlatFile.h"
/* #include "../../../cpp_wrapper/OpenAddressFileHash.h" */
#include "../../../cpp_wrapper/OpenAddressHash.h"
#include "../../../cpp_wrapper/SkipList.h"

int
main(
) {
	/* The following is example test code using int keys and string values */
	Dictionary<int, string> *dict = new BppTree<int, string>(key_type_numeric_signed, sizeof(int), 10);

	printf("Insert %d [%s]\n", 3, "hello!");

	string s = "hello!";

	dict->insert(3, s);

	string buf = dict->get(3);

	cout << "Retrieve key 3 and got back " << buf << endl << endl;

	cout << "Updating value of key 3 to hi!" << endl;

	string t = "hi!";

	dict->update(3, t);

	string buf2 = dict->get(3);

	cout << "Retrieve key 3 after update and got back " << buf2 << endl << endl;

	cout << "Delete key 3" << endl;
	dict->deleteRecord(3);

	cout << "Try and retrieve key 3 after deletion" << endl;

	string buf3 = dict->get(3);

	cout << "Retrieve key 3 and got back status " << (int) dict->last_status.error << endl << endl;

	printf("Insert %d [%s]\n", 3, "test1!");

	string u = "test1!";

	dict->insert(3, u);

	printf("Insert %d [%s]\n", 3, "test2!");

	string v = "test2!";

	dict->insert(3, v);

	printf("Insert %d [%s]\n", 4, "test3!");

	string w = "test3!";

	dict->insert(4, w);

	cout << "Testing equality query on key 3: " << endl;
	Cursor<int, string> *eq_cursor = dict->equality(3);

	while (eq_cursor->next()) {
		cout << "[eq] Got back [" << eq_cursor->getKey() << ", " << eq_cursor->getValue() << "]" << endl;
	}

	cout << endl;

	delete eq_cursor;

	cout << "Testing all records query: " << endl;
	Cursor<int, string> *all_cursor = dict->allRecords();

	while (all_cursor->next()) {
		cout << "[all] Got back [" << all_cursor->getKey() << ", " << all_cursor->getValue() << "]" << endl;
	}

	cout << endl;

	delete all_cursor;

	cout << "Testing range query: 2<=key<=3 " << endl;
	Cursor<int, string> *range_cursor = dict->range(2, 3);

	while (range_cursor->next()) {
		cout << "[range] Got back [" << range_cursor->getKey() << ", " << range_cursor->getValue() << "]" << endl;
	}

	cout << endl;

	delete range_cursor;

	delete dict;
}
