//
// Created by Dana Klamut on 2016-05-31.
//

#include <iostream>

#include "../Dictionary.h"
#include "../BppTree.h"

using namespace std;

int main() {
    Dictionary<int> *btree = new BppTree<int>(key_type_numeric_signed, sizeof(int), 10);

	printf("Insert %d [%s]\n", 3, "hello!");
	string s = "hello!";
	btree->insert(3, s);

	string buf = btree->get<string>(3);
	cout << "Retrieve key 3 and got back " << buf << endl;

	cout << "Updating value of key 3 to hi!" << endl;

	string t = "hi!";
	btree->update(3, t);

	string buf2 = btree->get<string>(3);
	cout << "Retrieve key 3 after update and got back " << buf2 << endl;

	cout << "Delete key 3" << endl;
	btree->deleteRecord(3);

	cout << "Try and retrieve key 3 after deletion" << endl;

	string buf3 = btree->get<string>(3);
	cout << "Retrieve key 3 and got back " << buf3 << endl;

	printf("Insert %d [%s]\n", 3, "test1!");
	string u = "test1!";
	btree->insert(3, u);

	printf("Insert %d [%s]\n", 3, "test2!");
	string v = "test2!";
	btree->insert(3, v);

	printf("Insert %d [%s]\n", 4, "test3!");
	string w = "test3!";
	btree->insert(4, w);

	cout << "Testing equality query on key 3: " << endl;
	btree->equality(3);

	/** Some issues, almost functioning */
	cout << "Testing all records query: " <<endl;
	btree->allRecords();

	/** Not functioning */
	//cout << "Testing range query: 2<key<5 " << endl;
	//btree->range(2, 5);

	cout << "End cpp wrapper test." << endl;

	btree->destroy();
	delete btree;
}