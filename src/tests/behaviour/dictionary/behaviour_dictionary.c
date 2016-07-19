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

/* This is a private struct we use to track metadata about the dictionary. */
bhdct_context_t bhdct_context = { NULL };

/**
@brief	This function binds the context properly. The context dictates what type of dictionary
		we're testing, so that these tests may be re-used across several implementations.
*/
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

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_type, dict->instance->key_type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_size, dict->instance->record.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value_size, dict->instance->record.value_size);
}

/**
@brief	This function initializes the master table.
*/
void
bhdct_master_table_init(
	planck_unit_test_t *tc
) {
	err_t err = ion_init_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
@brief	This function un-registers a dictionary from the master table.
*/
void
bhdct_delete_from_master_table(
	planck_unit_test_t	*tc,
	dictionary_t		*dict
) {
	err_t err = ion_delete_from_master_table(dict);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
@brief	This function deletes a dictionary.
*/
void
bhdct_delete_dictionary(
	planck_unit_test_t	*tc,
	dictionary_t		*dict
) {
	err_t err = dictionary_delete_dictionary(dict);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == dict->instance);
}

/**
@brief	This function closes the master table.
*/
void
bhdct_close_master_table(
	planck_unit_test_t *tc
) {
	err_t err = ion_close_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
@brief	This function deletes the master table.
*/
void
bhdct_delete_master_table(
	planck_unit_test_t *tc
) {
	err_t err = ion_delete_master_table();

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
	bhdct_master_table_init(tc);
	bhdct_context.init_fcn(handler);
	bhdct_dictionary_initialization(tc, handler, dict, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
}

/**
@brief	This function tears down a test case and cleans everything up.
*/
void
bhdct_takedown(
	planck_unit_test_t	*tc,
	dictionary_t		*dict
) {
/*	bhdct_delete_from_master_table(tc, dict); FIXME change when master table is fixed */
	bhdct_delete_dictionary(tc, dict);

	bhdct_close_master_table(tc);
	bhdct_delete_master_table(tc);
}

/**
@brief	This function does an insert into a dictionary.
*/
void
bhdct_insert(
	planck_unit_test_t	*tc,
	dictionary_t		*dict,
	ion_key_t			key,
	ion_value_t			value,
	boolean_t			check_result
) {
	ion_status_t status = dictionary_insert(dict, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	if (check_result) {
		ion_value_t retval = alloca(dict->instance->record.value_size);

		status = dictionary_get(dict, key, retval);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, memcmp(value, retval, dict->instance->record.value_size));
	}
}

/* =================================================== TEST CASES =================================================== */

/**
@brief	This function tests whether or not we can build and teardown a dictionary.
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

/**
@brief	This function tests a single insertion into a dictionary.
*/
void
test_bhdct_single_insert(
	planck_unit_test_t *tc
) {
	dictionary_handler_t	handler;
	dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict);

	bhdct_insert(tc, &dict, IONIZE(10, int), IONIZE(20, int), boolean_false);

	bhdct_takedown(tc, &dict);
}

planck_unit_suite_t *
bhdct_getsuite(
	void
) {
	if (NULL == bhdct_context.init_fcn) {
		return NULL;
	}

	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_setup);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_single_insert);

	return suite;
}
