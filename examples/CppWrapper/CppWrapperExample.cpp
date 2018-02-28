/******************************************************************************/
/**
@file		CppWrapperExample.cpp
@author		Dana Klamut
@brief		Example class exemplifying uses for Dictionary.h and MasterTable.h.
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
#include "../../src/cpp_wrapper/Dictionary.h"
#include "../../src/cpp_wrapper/MasterTable.h"

int
main(
	void
) {
	int key									= 0;
	int count								= 0;

	/* Create dictionary */
	ion_dictionary_id_t				id		= 1;
	ion_dictionary_config_info_t	config	= {
		.id = id, .type = key_type_numeric_signed, .key_size = sizeof(int), .value_size = 40, .dictionary_size = 20, .dictionary_type = dictionary_type_skip_list_t
	};
	int								type	= 0;
	string							val		= "";

	/* Traditional method of creating a dictionary */
/*	Dictionary<int, string> *dictionary = new SkipList<int, string>(id, key_type_numeric_signed, sizeof(int), sizeof("abracadabra"), 7); */

/*	Dictionary<int, string> *dictionary = SkipList<int, string>::open(config, type, val); */
	SkipList<int, string> *dictionary = SkipList<int, string>::openDictionary(config, type, val);

	if (err_ok != dictionary->last_status.error) {
		printf("Oh no! Something went wrong opening my dictionary\n");
	}

	string input;

	while (7 > count) {
		printf("READY FOR INPUT\n");
		getline(cin, input);

		printf("key: %i\n", key);
		cout << input << "\n";
		dictionary->insert(key, input);

		if (key > 0) {
			printf("last key: %i", key - 1);
			cout << "value: " << dictionary->get(key - 1) << "\n";
		}

		if (err_ok != dictionary->last_status.error) {
			printf("Oh no! Something went wrong opening my dictionary\n");
		}

		key = key + 1;

		Cursor<int, string> *cur = dictionary->allRecords();
		printf("\n<CURRENT DICTIONARY VALUES>\n");

		while (cur->next()) {
			printf("%i ", cur->getKey());
			cout << cur->getValue() << "\n";
		}

		printf("</CURRENT DICTIONARY VALUES>\n\n");
		delete cur;

		count++;
	}

	return 0;
}
