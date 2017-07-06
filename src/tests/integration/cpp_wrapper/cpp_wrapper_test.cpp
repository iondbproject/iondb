/******************************************************************************/
/**
@file		CppWrapperTest.cpp
@author		Dana Klamut
@brief		Test class exemplifying uses for Dictionary.h.
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

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#include <iostream>

using namespace std;

#include "../../../cpp_wrapper/Dictionary.h"
/*#include "../../../cpp_wrapper/BppTree.h"*/
/*#include "../../../cpp_wrapper/SkipList.h"*/
#include "../../../cpp_wrapper/FlatFile.h"
/* #include "../../../cpp_wrapper/OpenAddressFileHash.h" */
/*#include "../../../cpp_wrapper/OpenAddressHash.h"*/

int
main(
) {
	/* The following is example test code using int keys and string values */
	Dictionary<int, string> *dict = new FlatFile<int, string>(0, key_type_numeric_signed, sizeof(int), 10, 30);

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
