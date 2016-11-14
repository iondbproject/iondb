/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Behaviour tests for the B+ Tree implementation.
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
#include "../behaviour_dictionary.h"
#include "../../../../dictionary/bpp_tree/bpp_tree_handler.h"
#include "test_behaviour_bpp_tree.h"

void
runalltests_behaviour_bpp_tree(
	void
) {
	bhdct_run_tests(bpptree_init, -1, ION_BHDCT_ALL_TESTS & ~ION_BHDCT_STRING_INT);
}
