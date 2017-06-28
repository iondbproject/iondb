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

#include "../../src/cpp_wrapper/Dictionary.h"
#include "../../src/cpp_wrapper/MasterTable.h"

int
main(
	void
) {
	/* Create master table instance */
	MasterTable *master_table = new MasterTable();

	/* Perform master table initialization */
	master_table->initializeMasterTable();

	/**************************************************************************/

	/* Create Skip List dictionary using master table */

	/* Data type to initialize Key and Value type of dictionary being created */
	int type							= 0;

	Dictionary<int, int> *dictionary	= master_table->initializeDictionary(key_type_numeric_signed, type, type, sizeof(int), 10, 14, dictionary_type_skip_list_t);

	ion_dictionary_id_t dictionary_id	= dictionary->dict.instance->id;

	/**************************************************************************/

	/* Close dictionary before closing master table  or ensure dictionary
	instance has been closed previously to initialize it. */

	master_table->closeDictionary(dictionary);

	master_table->openDictionary(dictionary, dictionary_id);

	/**************************************************************************/

	/* Close master table - which will concurrently close dictionary */

	master_table->closeAllMasterTable();

	/**************************************************************************/

	/* Re-open master table */

	master_table->initializeMasterTable();

	/**************************************************************************/

	/* Re-open dictionary */

	master_table->openDictionary(dictionary, dictionary_id);

	/**************************************************************************/

	/* Delete dictionary */

	master_table->deleteDictionary(dictionary);

	/**************************************************************************/

	/* Test close master table - dictionary has been deleted so no need to close all */

	master_table->closeMasterTable();

	/**************************************************************************/

	/* Master table must be deleted using destructor */

	delete master_table;

	return 0;
}
