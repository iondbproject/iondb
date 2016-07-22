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

/* This is used to define how complicated to pre-fill a dictionary for testing. */
typedef enum {
	ion_fill_none, ion_fill_low, ion_fill_medium, ion_fill_high, ion_fill_edge_cases
} ion_behaviour_fill_level_e;

/* This is a private struct we use to track metadata about the dictionary. */
ion_bhdct_context_t bhdct_context = { NULL };

/**
@brief	This function binds the context properly. The context dictates what type of dictionary
		we're testing, so that these tests may be re-used across several implementations.
*/
void
bhdct_set_context(
	void (*init_fcn)(ion_dictionary_handler_t *)
) {
	bhdct_context.init_fcn = init_fcn;
}

/**
@brief	This function performs the dictionary initialization.
*/
void
bhdct_dictionary_initialization(
	planck_unit_test_t			*tc,
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dict,
	ion_key_type_t				key_type,
	int							key_size,
	int							value_size,
	int							dictionary_size
) {
	ion_err_t err = ion_master_table_create_dictionary(handler, dict, key_type, key_size, value_size, dictionary_size);

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
	ion_err_t err = ion_init_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
@brief	This function un-registers a dictionary from the master table.
*/
void
bhdct_delete_from_master_table(
	planck_unit_test_t	*tc,
	ion_dictionary_t	*dict
) {
	ion_err_t err = ion_delete_from_master_table(dict);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
@brief	This function deletes a dictionary.
*/
void
bhdct_delete_dictionary(
	planck_unit_test_t	*tc,
	ion_dictionary_t	*dict
) {
	ion_err_t err = dictionary_delete_dictionary(dict);

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
	ion_err_t err = ion_close_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
@brief	This function deletes the master table.
*/
void
bhdct_delete_master_table(
	planck_unit_test_t *tc
) {
	ion_err_t err = ion_delete_master_table();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

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

/**
@brief	This function performs a get on a dictionary.
*/
void
bhdct_get(
	planck_unit_test_t	*tc,
	ion_dictionary_t	*dict,
	ion_key_t			key,
	ion_value_t			expected_value,
	ion_err_t			expected_status,
	ion_result_count_t	expected_count
) {
	ion_value_t defaultval = alloca(dict->instance->record.value_size);

	memset(defaultval, 0x76, dict->instance->record.value_size);

	ion_value_t retval = alloca(dict->instance->record.value_size);

	memcpy(retval, defaultval, dict->instance->record.value_size);

	ion_status_t status = dictionary_get(dict, key, retval);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (err_ok == status.error) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, memcmp(expected_value, retval, dict->instance->record.value_size));
	}
	else {
		/* Here, we check to see that the passed in space to write the value remains unchanged, if we have an error condition. */
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, memcmp(defaultval, retval, dict->instance->record.value_size));
	}
}

/**
@brief	This function performs the setup required for a test case.
*/
void
bhdct_setup(
	planck_unit_test_t			*tc,
	ion_dictionary_handler_t	*handler,
	ion_dictionary_t			*dict,
	ion_behaviour_fill_level_e	fill_level
) {
	bhdct_master_table_init(tc);
	bhdct_context.init_fcn(handler);
	bhdct_dictionary_initialization(tc, handler, dict, key_type_numeric_signed, sizeof(int), sizeof(int), 10);

	/* This switch statement intentionally doesn't have breaks - we want it to fall through. */
	int i;

	switch (fill_level) {
		case ion_fill_edge_cases: {
			for (i = -100; i < -50; i += 2) {
				bhdct_insert(tc, dict, IONIZE(i, int), IONIZE(i * 3, int), boolean_true);
			}
		}

		case ion_fill_high: {
			for (i = 500; i < 1000; i += 5) {
				bhdct_insert(tc, dict, IONIZE(i, int), IONIZE(i * 10, int), boolean_true);
			}
		}

		case ion_fill_medium: {
			for (i = 50; i < 100; i += 2) {
				bhdct_insert(tc, dict, IONIZE(i, int), IONIZE(i * 5, int), boolean_true);
			}
		}

		case ion_fill_low: {
			for (i = 0; i < 10; i++) {
				bhdct_insert(tc, dict, IONIZE(i, int), IONIZE(i * 2, int), boolean_true);
			}
		}

		case ion_fill_none: {
			/* Intentionally left blank */
		}
	}
}

/**
@brief	This function tears down a test case and cleans everything up.
*/
void
bhdct_takedown(
	planck_unit_test_t	*tc,
	ion_dictionary_t	*dict
) {
/*	bhdct_delete_from_master_table(tc, dict); FIXME change when master table is fixed */
	bhdct_delete_dictionary(tc, dict);

	bhdct_close_master_table(tc);
	bhdct_delete_master_table(tc);
}

/* =================================================== TEST CASES =================================================== */

/**
@brief	This function tests whether or not we can build and teardown a dictionary.
*/
void
test_bhdct_setup(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);
	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests a single insertion into a dictionary.
*/
void
test_bhdct_insert_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(10, int), IONIZE(20, int), boolean_false);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests multiple insertions into a dictionary.
*/
void
test_bhdct_insert_multiple(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	int i;

	for (i = 50; i < 55; i++) {
		bhdct_insert(tc, &dict, IONIZE(i, int), IONIZE(i * 2, int), boolean_false);
	}

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests a retrieval on a dictionary that only has one record in it.
*/
void
test_bhdct_get_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(99, int), IONIZE(99 * 2, int), boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests retrieval on a dictionary with many records in it.
*/
void
test_bhdct_get_in_many(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(35, int), IONIZE(35 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(1002, int), IONIZE(1002 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(55, int), IONIZE(55 * 2, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(-5, int), IONIZE(-5 * 2, int), boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests retrieval on a dictionary with a whole bunch of records in it.
*/
void
test_bhdct_get_lots(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	int i;

	for (i = 300; i < 1000; i += 15) {
		bhdct_insert(tc, &dict, IONIZE(i, int), IONIZE(i * 5, int), boolean_true);
	}

	for (i = 300; i < 1000; i += 15) {
		bhdct_get(tc, &dict, IONIZE(i, int), IONIZE(i * 5, int), err_ok, 1);
	}

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests retrieval on an empty dictionary for a key that doesn't exist.
*/
void
test_bhdct_get_nonexist_empty(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_get(tc, &dict, IONIZE(99, int), NULL, err_item_not_found, 0);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests retrieval on a dictionary for a single key that doesn't exist.
*/
void
test_bhdct_get_nonexist_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_low);

	bhdct_get(tc, &dict, IONIZE(99, int), NULL, err_item_not_found, 0);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests retrieval on a dictionary for many keys that don't exist.
*/
void
test_bhdct_get_nonexist_many(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_edge_cases);

	bhdct_get(tc, &dict, IONIZE(-2000, int), NULL, err_item_not_found, 0);
	bhdct_get(tc, &dict, IONIZE(3000, int), NULL, err_item_not_found, 0);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests retrieval on a dictionary that has one record in it.
*/
void
test_bhdct_get_exist_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(30, int), IONIZE(30, int), boolean_true);
	/* FIXME: Intentional error here */
	bhdct_get(tc, &dict, IONIZE(30, int), IONIZE(000, int), err_ok, 1);

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
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_insert_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_insert_multiple);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_in_many);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_lots);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_nonexist_empty);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_nonexist_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_nonexist_many);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_exist_single);

	return suite;
}
