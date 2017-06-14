/******************************************************************************/
/**
@file
@author		Spencer MacBeth
@brief		Behaviour tests for the flat file implementation.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
*/
/******************************************************************************/

#include "../../../planck-unit/src/planck_unit.h"
#include "../behaviour_dictionary.h"
#include "../../../../dictionary/linear_hash/linear_hash_handler.h"
#include "test_behaviour_linear_hash.h"

void
runalltests_behaviour_linear_hash(
	void
) {
	bhdct_run_tests(linear_hash_dict_init, 15, ION_BHDCT_ALL_TESTS);
}
