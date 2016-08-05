/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Implementation dictionary level unit tests for the flat file store.
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

#include "test_flat_file_dictionary_handler.h"

planck_unit_suite_t *
flat_file_handler_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	return suite;
}

void
runalltests_flat_file_handler(
) {
	planck_unit_suite_t *suite = flat_file_handler_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
