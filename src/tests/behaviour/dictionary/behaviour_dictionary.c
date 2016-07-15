/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Behaviour tests for all dictionary implementations.
@details	The behaviour tests represent a "black box" approach to the
			dictionary implementations. These tests simply assert that the
			output obtained given specific inputs is as we expect. The expectation
			is that more exhaustive testing is done at the unit level in order to
			assert that the implementations themselves behave as expected.
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

#include "behaviour_dictionary.h"

bhdct_context_t bhdct_context;

void
bhdct_set_context(
	void (*init_fcn)(dictionary_handler_t *)
) {
	bhdct_context.init_fcn = init_fcn;
}

/**
@brief	This function performs the dictionary initialization.
*/
void
bhdct_dictionary_initialization(
	planck_unit_test_t		*tc,
	dictionary_handler_t	*handler,
	dictionary_t			*dict,
	key_type_t				key_type,
	int						key_size,
	int						value_size,
	int						dictionary_size
) {
	err_t err = ion_master_table_create_dictionary(handler, dict, key_type, key_size, value_size, dictionary_size);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
@brief	This function performs the setup required for a test case.
*/
void
bhdct_setup(
	planck_unit_test_t		*tc,
	dictionary_handler_t	*handler,
	dictionary_t			*dict
) {
	err_t err = ion_init_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	bhdct_context.init_fcn(handler);
	bhdct_dictionary_initialization(tc, handler, dict, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
}

/**
@brief	This function performs the setup required for a test case.
*/
void
bhdct_takedown(
	planck_unit_test_t	*tc,
	dictionary_t		*dict
) {
/*	err_t err = dictionary_delete_dictionary(dict); // todo: this isn't needed once master table properly tracks dictionaries */
/*	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err); // FIXME: This has to be deleted one way or another */

	err_t err = ion_delete_from_master_table(dict);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	err = ion_close_master_table();
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	err = ion_delete_master_table();
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
@brief	This function conducts a test on dictionary setup.
*/
void
test_bhdct_setup(
	planck_unit_test_t *tc
) {
	dictionary_handler_t	handler;
	dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict);

	bhdct_takedown(tc, &dict);
}

planck_unit_suite_t *
bhdct_getsuite(
	void
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_setup);

	return suite;
}
