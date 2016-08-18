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

	planck_unit_suite_t *suite_1	= bhdct_getsuite_bpptree_1();
	planck_unit_suite_t *suite_2	= bhdct_getsuite_bpptree_2();
	planck_unit_suite_t *suite_3	= bhdct_getsuite_bpptree_3();
/*	planck_unit_suite_t *suite_4	= bhdct_getsuite_bpptree_4(); */
	planck_unit_suite_t *suite_5	= bhdct_getsuite_bpptree_5();
	planck_unit_suite_t *suite_6	= bhdct_getsuite_bpptree_6();
	planck_unit_suite_t *suite_7	= bhdct_getsuite_bpptree_7();
	planck_unit_suite_t *suite_8	= bhdct_getsuite_bpptree_8();
	planck_unit_suite_t *suite_9	= bhdct_getsuite_bpptree_9();
	planck_unit_suite_t *suite_10	= bhdct_getsuite_bpptree_10();
	planck_unit_suite_t *suite_11	= bhdct_getsuite_bpptree_11();

	planck_unit_run_suite(suite_1);
	planck_unit_destroy_suite(suite_1);

	planck_unit_run_suite(suite_2);
	planck_unit_destroy_suite(suite_2);

	planck_unit_run_suite(suite_3);
	planck_unit_destroy_suite(suite_3);

/*	planck_unit_run_suite(suite_4); */
/*	planck_unit_destroy_suite(suite_4); */

	planck_unit_run_suite(suite_5);
	planck_unit_destroy_suite(suite_5);

	planck_unit_run_suite(suite_6);
	planck_unit_destroy_suite(suite_6);

	planck_unit_run_suite(suite_7);
	planck_unit_destroy_suite(suite_7);

	planck_unit_run_suite(suite_8);
	planck_unit_destroy_suite(suite_8);

	planck_unit_run_suite(suite_9);
	planck_unit_destroy_suite(suite_9);

	planck_unit_run_suite(suite_10);
	planck_unit_destroy_suite(suite_10);

	planck_unit_run_suite(suite_11);
	planck_unit_destroy_suite(suite_11);
}
