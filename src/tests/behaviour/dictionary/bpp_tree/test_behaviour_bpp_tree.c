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
	bhdct_set_context(bpptree_init, -1, boolean_true);

	planck_unit_suite_t *suite1 = bhdct_getsuite_bpptree1();

	planck_unit_run_suite(suite1);
	planck_unit_destroy_suite(suite1);

	planck_unit_suite_t *suite2 = bhdct_getsuite_bpptree2();

	planck_unit_run_suite(suite2);
	planck_unit_destroy_suite(suite2);

	planck_unit_suite_t *suite3 = bhdct_getsuite_bpptree3();

	planck_unit_run_suite(suite3);
	planck_unit_destroy_suite(suite3);

	planck_unit_suite_t *suite4 = bhdct_getsuite_bpptree4();

	planck_unit_run_suite(suite4);
	planck_unit_destroy_suite(suite4);

	planck_unit_suite_t *suite5 = bhdct_getsuite_bpptree5();

	planck_unit_run_suite(suite5);
	planck_unit_destroy_suite(suite5);

	planck_unit_suite_t *suite6 = bhdct_getsuite_bpptree6();

	planck_unit_run_suite(suite6);
	planck_unit_destroy_suite(suite6);

	planck_unit_suite_t *suite7 = bhdct_getsuite_bpptree7();

	planck_unit_run_suite(suite7);
	planck_unit_destroy_suite(suite7);

	planck_unit_suite_t *suite8 = bhdct_getsuite_bpptree8();

	planck_unit_run_suite(suite8);
	planck_unit_destroy_suite(suite8);
}
