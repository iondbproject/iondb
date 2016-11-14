/******************************************************************************/
/**
@file
@author		Kris Wallperington
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

#include "../../../planckunit/src/planck_unit.h"
#include "../behaviour_dictionary_cursor.h"
#include "../../../../dictionary/flat_file/flat_file_dictionary_handler.h"
#include "test_behaviour_flat_file.h"

void
runalltests_cursor_behaviour_flat_file(
	void
) {
	bhdct_run_cursor_tests(ffdict_init, 15, ION_BHDCT_ALL_TESTS);
}
