/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Behaviour tests for the skip list implementation.
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
#include "../../../../dictionary/skip_list/skip_list_handler.h"
#include "test_behaviour_skip_list.h"

void
runalltests_behaviour_skip_list(
	void
) {
	bhdct_set_context(sldict_init, 7, boolean_true);

	planck_unit_suite_t *suite_1	= bhdct_getsuite();
	planck_unit_suite_t *suite_2	= bhdct_getsuite_2();

	planck_unit_run_suite(suite_1);
	planck_unit_destroy_suite(suite_1);

/*	planck_unit_run_suite(suite_2); */
/*	planck_unit_destroy_suite(suite_2); */
}
