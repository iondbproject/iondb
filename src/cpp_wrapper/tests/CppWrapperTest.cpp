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

	btree->destroy();
	delete btree;
}