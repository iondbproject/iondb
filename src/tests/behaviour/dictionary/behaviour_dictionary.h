/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Entry point for dictionary level behaviour tests.
@details	Note that the tests assume a minimum capacity of 160 simultaneous
			records being present in the dictionary at any given time. A max
			capacity of 200 is suggested.
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

#if !defined(TEST_BEHAVIOUR_H)
#define TEST_BEHAVIOUR_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include "../../planckunit/src/planck_unit.h"
#include "../../../dictionary/dictionary_types.h"
#include "../../../dictionary/dictionary.h"
#include "../../../dictionary/ion_master_table.h"

typedef struct {
	void (*init_fcn)(
		ion_dictionary_handler_t *
	);										/**< A pointer to a handler initialization function. */

	ion_dictionary_size_t	dictionary_size;/**< Some configuration for the dictionary, so that we can change parameters
												   on a per-implementation basis. */
	ion_boolean_t			duplicate_support;	/**< Tells whether or not the implementation being tested supports duplicates. */
} ion_bhdct_context_t;

/**
@brief	This function specifies an information context. Used by the concrete test runners
		in order to properly designate which implementation we're specifically testing.
*/
void bhdct_set_context(void (*)(ion_dictionary_handler_t *), ion_dictionary_size_t, ion_boolean_t);

/**
@brief	Constructs the testing suite for the behaviour dictionary tests.
@return	A constructed, initialized suite.
*/
planck_unit_suite_t *
bhdct_getsuite(
	void
);

#if defined(__cplusplus)
}
#endif

#endif
