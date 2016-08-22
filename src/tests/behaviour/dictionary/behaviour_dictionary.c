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

#define ION_FILL_LOW_LOOP(var) \
	for (var = 0; var < 10; var++)
#define ION_FILL_MEDIUM_LOOP(var) \
	for (var = 50; var < 100; var += 2)
#define ION_FILL_HIGH_LOOP(var) \
	for (var = 500; var < 1000; var += 5)
#define ION_FILL_EDGE_LOOP(var) \
	for (var = -100; var < -50; var += 2)

#define ION_LOW_VALUE(var)		IONIZE((var) * 2, int)
#define ION_MEDIUM_VALUE(var)	IONIZE((var) * 5, int)
#define ION_HIGH_VALUE(var)		IONIZE((var) * 10, int)
#define ION_EDGE_VALUE(var)		IONIZE((var) * 3, int)

/* This is a private struct we use to track metadata about the dictionary. */
ion_bhdct_context_t bhdct_context = { 0 };

/**
@brief	This function binds the context properly. The context dictates what type of dictionary
		we're testing, so that these tests may be re-used across several implementations.
*/
void
bhdct_set_context(
	ion_handler_initializer_t	init_fcn,
	ion_dictionary_size_t		dictionary_size,
	ion_boolean_t				duplicate_support
) {
	bhdct_context.init_fcn			= init_fcn;
	bhdct_context.dictionary_size	= dictionary_size;
	bhdct_context.duplicate_support = duplicate_support;
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
		bhdct_get(tc, dict, key, value, err_ok, 1);
	}
}

/**
@brief	This function performs a delete on a dictionary.
*/
void
bhdct_delete(
	planck_unit_test_t	*tc,
	ion_dictionary_t	*dict,
	ion_key_t			key,
	ion_err_t			expected_status,
	ion_result_count_t	expected_count,
	ion_boolean_t		check_result
) {
	ion_status_t status = dictionary_delete(dict, key);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (check_result) {
		bhdct_get(tc, dict, key, NULL, err_item_not_found, 0);
	}
}

/**
@brief	This function performs an update on a dictionary.
*/
void
bhdct_update(
	planck_unit_test_t	*tc,
	ion_dictionary_t	*dict,
	ion_key_t			key,
	ion_value_t			value,
	ion_err_t			expected_status,
	ion_result_count_t	expected_count,
	ion_boolean_t		check_result
) {
	ion_status_t status = dictionary_update(dict, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (check_result) {
		bhdct_get(tc, dict, key, value, err_ok, 1);
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
	/* Note most of these are fixed except the dictionary size */
	bhdct_dictionary_initialization(tc, handler, dict, key_type_numeric_signed, sizeof(int), sizeof(int), bhdct_context.dictionary_size);

	/* This switch statement intentionally doesn't have breaks - we want it to fall through. */
	int i;

	switch (fill_level) {
		case ion_fill_edge_cases: {
			ION_FILL_EDGE_LOOP(i) {
				bhdct_insert(tc, dict, IONIZE(i, int), ION_EDGE_VALUE(i), boolean_true);
			}
		}

		case ion_fill_high: {
			ION_FILL_HIGH_LOOP(i) {
				bhdct_insert(tc, dict, IONIZE(i, int), ION_HIGH_VALUE(i), boolean_true);
			}
		}

		case ion_fill_medium: {
			ION_FILL_MEDIUM_LOOP(i) {
				bhdct_insert(tc, dict, IONIZE(i, int), ION_MEDIUM_VALUE(i), boolean_true);
			}
		}

		case ion_fill_low: {
			ION_FILL_LOW_LOOP(i) {
				bhdct_insert(tc, dict, IONIZE(i, int), ION_LOW_VALUE(i), boolean_true);
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
		We search for a key that exists, and expect that we get a positive response back.
*/
void
test_bhdct_get_exist_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(30, int), IONIZE(30, int), boolean_true);
	bhdct_get(tc, &dict, IONIZE(30, int), IONIZE(30, int), err_ok, 1);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests retrieval on a dictionary that has many records in it.
		We expect a positive result.
*/
void
test_bhdct_get_populated_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_low);

	bhdct_insert(tc, &dict, IONIZE(92, int), IONIZE(92, int), boolean_true);
	bhdct_get(tc, &dict, IONIZE(92, int), IONIZE(92, int), err_ok, 1);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests retrieval on a dictionary that has many records in it.
		We expect a positive result on all gets run.
*/
void
test_bhdct_get_populated_multiple(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_low);

	int i;

	ION_FILL_LOW_LOOP(i) {
		bhdct_get(tc, &dict, IONIZE(i, int), ION_LOW_VALUE(i), err_ok, 1);
	}

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests a get of everything within a dictionary.
*/
void
test_bhdct_get_all(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_edge_cases);

	int i;

	ION_FILL_LOW_LOOP(i) {
		bhdct_get(tc, &dict, IONIZE(i, int), ION_LOW_VALUE(i), err_ok, 1);
	}
	ION_FILL_MEDIUM_LOOP(i) {
		bhdct_get(tc, &dict, IONIZE(i, int), ION_MEDIUM_VALUE(i), err_ok, 1);
	}
	ION_FILL_HIGH_LOOP(i) {
		bhdct_get(tc, &dict, IONIZE(i, int), ION_HIGH_VALUE(i), err_ok, 1);
	}
	ION_FILL_EDGE_LOOP(i) {
		bhdct_get(tc, &dict, IONIZE(i, int), ION_EDGE_VALUE(i), err_ok, 1);
	}

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests deletion on an empty dictionary.
		We expect to receive err_item_not_found and for everything to remain as-is.
*/
void
test_bhdct_delete_empty(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_delete(tc, &dict, IONIZE(3, int), err_item_not_found, 0, boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests deletion on a dictionary that has one element,
		but not the one we are looking for. We expect to receive err_item_not_found
		and for everything to remain as-is.
*/
void
test_bhdct_delete_nonexist_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(5, int), IONIZE(10, int), boolean_true);
	bhdct_delete(tc, &dict, IONIZE(3, int), err_item_not_found, 0, boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests deletion on a dictionary that has many elements,
		but not the one we are looking for. We expect to receive err_item_not_found
		and for everything to remain as-is.
*/
void
test_bhdct_delete_nonexist_several(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_medium);

	bhdct_delete(tc, &dict, IONIZE(-100, int), err_item_not_found, 0, boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests deletion on a dictionary that has a single element,
		which is the one we're looking for.
*/
void
test_bhdct_delete_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(3, int), IONIZE(6, int), boolean_true);
	bhdct_delete(tc, &dict, IONIZE(3, int), err_ok, 1, boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests deletion on a dictionary that has many elements,
		of which contains the one we choose to delete.
*/
void
test_bhdct_delete_single_several(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_high);

	bhdct_delete(tc, &dict, IONIZE(700, int), err_ok, 1, boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests deletion of everything within a dictionary.
*/
void
test_bhdct_delete_all(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_edge_cases);

	int i;

	ION_FILL_LOW_LOOP(i) {
		bhdct_delete(tc, &dict, IONIZE(i, int), err_ok, 1, boolean_true);
	}
	ION_FILL_MEDIUM_LOOP(i) {
		bhdct_delete(tc, &dict, IONIZE(i, int), err_ok, 1, boolean_true);
	}
	ION_FILL_HIGH_LOOP(i) {
		bhdct_delete(tc, &dict, IONIZE(i, int), err_ok, 1, boolean_true);
	}
	ION_FILL_EDGE_LOOP(i) {
		bhdct_delete(tc, &dict, IONIZE(i, int), err_ok, 1, boolean_true);
	}

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an update on a dictionary that is empty. We expect an upsert to occur.
*/
void
test_bhdct_update_empty_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_update(tc, &dict, IONIZE(3, int), IONIZE(5, int), err_ok, 1, boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an update on a dictionary that has one element,
		but not the one we are looking for. We expect an upsert to occur.
*/
void
test_bhdct_update_nonexist_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(10, int), IONIZE(4, int), boolean_true);
	bhdct_update(tc, &dict, IONIZE(3, int), IONIZE(5, int), err_ok, 1, boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an update on a dictionary that has many elements,
		but not the one we are looking for. We expect an upsert to occur.
*/
void
test_bhdct_update_nonexist_in_many(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_medium);

	bhdct_update(tc, &dict, IONIZE(63, int), IONIZE(-10, int), err_ok, 1, boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an update on a dictionary that has one element,
		which is the one we are looking for.
*/
void
test_bhdct_update_exist_single(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_none);

	bhdct_insert(tc, &dict, IONIZE(23, int), IONIZE(0, int), boolean_true);
	bhdct_update(tc, &dict, IONIZE(23, int), IONIZE(44, int), err_ok, 1, boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests an update on a dictionary that has many elements,
		which includes the one we are looking for.
*/
void
test_bhdct_update_exist_in_many(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_medium);

	bhdct_update(tc, &dict, IONIZE(60, int), IONIZE(-23, int), err_ok, 1, boolean_true);

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests update of everything within a dictionary.
*/
void
test_bhdct_update_all(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_edge_cases);

	int i;

	ION_FILL_LOW_LOOP(i) {
		bhdct_update(tc, &dict, IONIZE(i, int), IONIZE(-1337, int), err_ok, 1, boolean_true);
	}
	ION_FILL_MEDIUM_LOOP(i) {
		bhdct_update(tc, &dict, IONIZE(i, int), IONIZE(-1337, int), err_ok, 1, boolean_true);
	}
	ION_FILL_HIGH_LOOP(i) {
		bhdct_update(tc, &dict, IONIZE(i, int), IONIZE(-1337, int), err_ok, 1, boolean_true);
	}
	ION_FILL_EDGE_LOOP(i) {
		bhdct_update(tc, &dict, IONIZE(i, int), IONIZE(-1337, int), err_ok, 1, boolean_true);
	}

	bhdct_takedown(tc, &dict);
}

/**
@brief	This function tests some deletes, followed by an insert. Nothing should go wrong.
*/
void
test_bhdct_delete_then_insert(
	planck_unit_test_t *tc
) {
	ion_dictionary_handler_t	handler;
	ion_dictionary_t			dict;

	bhdct_setup(tc, &handler, &dict, ion_fill_edge_cases);

	bhdct_delete(tc, &dict, IONIZE(60, int), err_ok, 1, boolean_true);
	bhdct_delete(tc, &dict, IONIZE(4, int), err_ok, 1, boolean_true);
	bhdct_delete(tc, &dict, IONIZE(505, int), err_ok, 1, boolean_true);

	bhdct_insert(tc, &dict, IONIZE(61, int), IONIZE(44, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(67, int), IONIZE(42, int), boolean_true);
	bhdct_insert(tc, &dict, IONIZE(73, int), IONIZE(48, int), boolean_true);

	bhdct_takedown(tc, &dict);
}

planck_unit_suite_t *
bhdct_getsuite_1(
	void
) {
	if (NULL == bhdct_context.init_fcn) {
		fprintf(stderr, "Behaviour Dictionary context was not set!");
		return NULL;
	}

	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_setup);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_insert_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_insert_multiple);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_in_many);
/*	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_lots); */

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_nonexist_empty);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_nonexist_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_nonexist_many);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_exist_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_populated_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_populated_multiple);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_get_all);

	return suite;
}

planck_unit_suite_t *
bhdct_getsuite_2(
	void
) {
	if (NULL == bhdct_context.init_fcn) {
		fprintf(stderr, "Behaviour Dictionary context was not set!");
		return NULL;
	}

	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_delete_empty);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_delete_nonexist_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_delete_nonexist_several);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_delete_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_delete_single_several);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_delete_all);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_update_empty_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_update_nonexist_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_update_nonexist_in_many);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_update_exist_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_update_exist_in_many);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_update_all);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_bhdct_delete_then_insert);

	return suite;
}
