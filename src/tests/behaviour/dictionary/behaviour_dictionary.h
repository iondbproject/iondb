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

#define ION_BHDCT_INT_INT					(1 << 0)
#define ION_BHDCT_STRING_INT				(1 << 1)
#define ION_BHDCT_DUPLICATES				(1 << 2)
#define ION_BHDCT_ALL_TESTS					(0xFFFFFFFF)

#define ION_BHDCT_STRING_KEY_BUFFER_SIZE	9
#define ION_BHDCT_STRING_KEY_PAYLOAD		"k%d"

typedef struct {
	ion_handler_initializer_t	init_fcn;		/**< A pointer to a handler initialization function. */

	ion_dictionary_size_t		dictionary_size;/**< Some configuration for the dictionary, so that we can change parameters
												   on a per-implementation basis. */
	uint32_t					test_classes;	/**< A bit mask that determines which sets of tests to run. */
} ion_bhdct_context_t;

/**
@brief	This function tests whether or not we can build and teardown a dictionary.
*/
void
test_bhdct_setup(
	planck_unit_test_t *tc
);

/**
@brief	This function tears down a test case and cleans everything up.
*/
void
bhdct_takedown(
	planck_unit_test_t	*tc,
	ion_dictionary_t	*dict
);

/**
@brief	This function does an insert into a dictionary.
*/
void
bhdct_insert(
	planck_unit_test_t	*tc,
	ion_dictionary_t	*dict,
	ion_key_t			key,
	ion_value_t			value,
	ion_boolean_t		check_result
);

/**
@brief		Executes the behaviour test suite, given the testing parameters.
@param		init_fcn
				A function pointer that designates the initializer for a specific dictionary implementation.
@param		dictionary_size
				The specified dictionary size to use for tests.
@param		test_classes
				A supplied bit mask used to determine which tests to run.
*/
void
bhdct_run_tests(
	ion_handler_initializer_t	init_fcn,
	ion_dictionary_size_t		dictionary_size,
	uint32_t					test_classes
);

#if defined(__cplusplus)
}
#endif

#endif
