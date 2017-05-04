/******************************************************************************/
/**
@file
@author		Kris Wallperington, Dana Klamut
@brief		Unit test for the C++ Wrapper
@todo		These tests need to be fully fleshed out.
*/
/******************************************************************************/

#include "../../planck-unit/src/planck_unit.h"
#include "../../../cpp_wrapper/Dictionary.h"
#include "../../../cpp_wrapper/BppTree.h"
#include "../../../cpp_wrapper/FlatFile.h"
#include "../../../cpp_wrapper/OpenAddressFileHash.h"
#include "../../../cpp_wrapper/OpenAddressHash.h"
#include "../../../cpp_wrapper/SkipList.h"
#include "test_cpp_wrapper.h"

/* This is used to define how complicated to pre-fill a dictionary for testing. */
typedef enum ION_BEHAVIOUR_FILL_LEVEL {
	ion_fill_none, ion_fill_low, ion_fill_medium, ion_fill_high, ion_fill_edge_cases
} ion_behaviour_fill_level_e;

#define ION_FILL_LOW_LOOP(var) \
	for (var = 0; var<10;var ++)
#define ION_FILL_MEDIUM_LOOP(var) \
	for (var = 50; var<100;var += 2)
#define ION_FILL_HIGH_LOOP(var) \
	for (var = 500; var<1000;var += 5)
#define ION_FILL_EDGE_LOOP(var) \
	for (var = -100; var<-50;var += 2)

/**
@brief	Tests the creation of a B+ Tree (arbitrarily chosen) dictionary and asserts
		the validity of some dictionary parameters.
*/
Dictionary<int, int> *
cpp_wrapper_create(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, dict->dict.instance->key_type == key_type_numeric_signed);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict->dict.instance->compare == dictionary_compare_signed_value);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict->dict.instance->record.key_size == sizeof(int));
	PLANCK_UNIT_ASSERT_TRUE(tc, dict->dict.instance->record.value_size == sizeof(int));

	return dict;
}

/**
@brief	Tests the creation of a B+ Tree (arbitrarily chosen) dictionary and asserts
		the validity of some dictionary parameters.
*/
void
cpp_wrapper_destroy(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	ion_err_t err = dict->destroy();

	delete dict;

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
@brief	This function performs a get on a dictionary.
*/
void
cpp_wrapper_get(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dict,
	int key,
	int expected_value,
	ion_err_t expected_status,
	ion_result_count_t expected_count
) {
	int				retval = dict->get(key);
	ion_status_t	status = dict->last_status;

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (err_ok == status.error) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_value, retval);
	}
}

/**
@brief	This function does an insert into a dictionary.
*/
void
cpp_wrapper_insert(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dict,
	int key,
	int value,
	ion_boolean_t check_result
) {
	ion_status_t status = dict->insert(key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	if (check_result) {
		cpp_wrapper_get(tc, dict, key, value, err_ok, 1);
	}
}

/**
@brief	This function performs a delete on a dictionary.
*/
void
cpp_wrapper_delete(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dict,
	int key,
	ion_err_t expected_status,
	ion_result_count_t expected_count,
	ion_boolean_t check_result
) {
	ion_status_t status = dict->deleteRecord(key);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (check_result) {
		cpp_wrapper_get(tc, dict, key, NULL, err_item_not_found, 0);
	}
}

/**
@brief	This function performs an update on a dictionary.
*/
void
cpp_wrapper_update(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dict,
	int key,
	int value,
	ion_err_t expected_status,
	ion_result_count_t expected_count,
	ion_boolean_t check_result
) {
	ion_status_t status = dict->update(key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (check_result) {
		cpp_wrapper_get(tc, dict, key, value, err_ok, 1);
	}
}

/**
@brief	This function performs the setup required for a test case.
*/
void
cpp_wrapper_setup(
	planck_unit_test_t *tc,
	Dictionary<int, int>			*dict,
	ion_behaviour_fill_level_e fill_level
) {
	/* This switch statement intentionally doesn't have breaks - we want it to fall through. */
	int i;

	switch (fill_level) {
		case ion_fill_edge_cases: {
			ION_FILL_EDGE_LOOP(i) {
				cpp_wrapper_insert(tc, dict, i, i * 3, boolean_true);
			}
		}

		case ion_fill_high: {
			ION_FILL_HIGH_LOOP(i) {
				cpp_wrapper_insert(tc, dict, i, i * 10, boolean_true);
			}
		}

		case ion_fill_medium: {
			ION_FILL_MEDIUM_LOOP(i) {
				cpp_wrapper_insert(tc, dict, i, i * 5, boolean_true);
			}
		}

		case ion_fill_low: {
			ION_FILL_LOW_LOOP(i) {
				cpp_wrapper_insert(tc, dict, i, i * 2, boolean_true);
			}
		}

		case ion_fill_none: {
			/* Intentionally left blank */
		}
	}
}

/* =================================================== TEST CASES =================================================== */

/**
@brief	This function tests whether or not we can build and teardown a dictionary.
*/
void
test_cpp_wrapper_create(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = cpp_wrapper_create(tc);
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests whether or not we can build and teardown a dictionary.
*/
void
test_cpp_wrapper_setup(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests whether or not we can build and teardown a dictionary
		on all dictionary implementations.
*/
void
test_cpp_wrapper_setup_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_setup(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_setup(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_setup(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_setup(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_setup(tc, dict);
}

/**
@brief	This function tests a single insertion into a dictionary.
*/
void
test_cpp_wrapper_insert_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 10, 20, boolean_false);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests a single insertion into a dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_insert_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_insert_single(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_insert_single(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_insert_single(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_single(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_single(tc, dict);
}

/**
@brief	This function tests multiple insertions into a dictionary.
*/
void
test_cpp_wrapper_insert_multiple(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);

	int i;

	for (i = 50; i < 55; i++) {
		cpp_wrapper_insert(tc, dict, i, i * 2, boolean_false);
	}

	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests multiple insertions into a dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_insert_multiple_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_insert_multiple(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_insert_multiple(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_insert_multiple(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_multiple(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_multiple(tc, dict);
}

/**
@brief	This function tests a retrieval on a dictionary that only has one record in it.
*/
void
test_cpp_wrapper_get_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 99, 99 * 2, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests a retrieval on a dictionary that only has one record
		in it on all dictionary implementations.
*/
void
test_cpp_wrapper_get_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_single(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_single(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_single(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_single(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_single(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary with many records in it.
*/
void
test_cpp_wrapper_get_in_many(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 35, 35 * 2, boolean_true);
	cpp_wrapper_insert(tc, dict, 1002, 1002 * 2, boolean_true);
	cpp_wrapper_insert(tc, dict, 55, 55 * 2, boolean_true);
	cpp_wrapper_insert(tc, dict, -5, -5 * 2, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary with many records in it
		on all dictionary implementations.
*/
void
test_cpp_wrapper_get_in_many_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_in_many(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_in_many(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_in_many(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_in_many(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_in_many(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary with a whole bunch of records in it.
*/
void
test_cpp_wrapper_get_lots(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);

	int i;

	for (i = 300; i < 1000; i += 15) {
		cpp_wrapper_insert(tc, dict, i, i * 5, boolean_true);
	}

	for (i = 300; i < 1000; i += 15) {
		cpp_wrapper_get(tc, dict, i, i * 5, err_ok, 1);
	}

	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary with a whole bunch of
		records in it on all dictionary implementations.
*/
void
test_cpp_wrapper_get_lots_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_lots(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_lots(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_lots(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_lots(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_lots(tc, dict);
}

/**
@brief	This function tests retrieval on an empty dictionary for a key that doesn't exist.
*/
void
test_cpp_wrapper_get_nonexist_empty(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_get(tc, dict, 99, NULL, err_item_not_found, 0);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests retrieval on an empty dictionary for a key that
		doesn't exist on all dictionary implementations.
*/
void
test_cpp_wrapper_get_nonexist_empty_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_nonexist_empty(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary for a single key that doesn't exist.
*/
void
test_cpp_wrapper_get_nonexist_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_low);
	cpp_wrapper_get(tc, dict, 99, NULL, err_item_not_found, 0);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary for a single key that
		doesn't exist on all implementations.
*/
void
test_cpp_wrapper_get_nonexist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_nonexist_single(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_nonexist_single(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_nonexist_single(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_single(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_single(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary for many keys that don't exist.
*/
void
test_cpp_wrapper_get_nonexist_many(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_edge_cases);
	cpp_wrapper_get(tc, dict, -2000, NULL, err_item_not_found, 0);
	cpp_wrapper_get(tc, dict, 3000, NULL, err_item_not_found, 0);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary for many keys that don't
		exist on all dictionary implementations.
*/
void
test_cpp_wrapper_get_nonexist_many_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_nonexist_many(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_nonexist_many(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_nonexist_many(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_many(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_many(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary that has one record in it.
		We search for a key that exists, and expect that we get a positive response back.
*/
void
test_cpp_wrapper_get_exist_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 30, 30, boolean_true);
	cpp_wrapper_get(tc, dict, 30, 30, err_ok, 1);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary that has one record in it
		on all dictionary implementations. We search for a key that exists, and
		expect that we get a positive response back.
*/
void
test_cpp_wrapper_get_exist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_exist_single(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_exist_single(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_exist_single(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_exist_single(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_exist_single(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary that has many records in it.
		We expect a positive result.
*/
void
test_cpp_wrapper_get_populated_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_low);
	cpp_wrapper_insert(tc, dict, 92, 92, boolean_true);
	cpp_wrapper_get(tc, dict, 92, 92, err_ok, 1);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary that has many records in it
		on all dictionary implementations. We expect a positive result.
*/
void
test_cpp_wrapper_get_populated_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_populated_single(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_populated_single(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_populated_single(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_single(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_single(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary that has many records in it.
		We expect a positive result on all gets run.
*/
void
test_cpp_wrapper_get_populated_multiple(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_low);

	int i;

	ION_FILL_LOW_LOOP(i) {
		cpp_wrapper_get(tc, dict, i, i * 2, err_ok, 1);
	}

	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests retrieval on a dictionary that has many records in it
		on all dictionary implementations. We expect a positive result on all
		gets run.
*/
void
test_cpp_wrapper_get_populated_multiple_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_populated_multiple(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_populated_multiple(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_populated_multiple(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_multiple(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_multiple(tc, dict);
}

/**
@brief	This function tests a get of everything within a dictionary.
*/
void
test_cpp_wrapper_get_all(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_edge_cases);

	int i;

	ION_FILL_LOW_LOOP(i) {
		cpp_wrapper_get(tc, dict, i, i * 2, err_ok, 1);
	}
	ION_FILL_MEDIUM_LOOP(i) {
		cpp_wrapper_get(tc, dict, i, i * 5, err_ok, 1);
	}
	ION_FILL_HIGH_LOOP(i) {
		cpp_wrapper_get(tc, dict, i, i * 10, err_ok, 1);
	}
	ION_FILL_EDGE_LOOP(i) {
		cpp_wrapper_get(tc, dict, i, i * 3, err_ok, 1);
	}

	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests a get of everything within a dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_get_all_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_all(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_all(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_all(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_all(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_all(tc, dict);
}

/**
@brief	This function tests deletion on an empty dictionary.
		We expect to receive err_item_not_found and for everything to remain as-is.
*/
void
test_cpp_wrapper_delete_empty(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_delete(tc, dict, 3, err_item_not_found, 0, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests deletion on an empty dictionary on all dictionary
		implementations. We expect to receive err_item_not_found and for everything
		else to remain as-is.
*/
void
test_cpp_wrapper_delete_empty_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_empty(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_empty(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_empty(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_empty(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_empty(tc, dict);
}

/**
@brief	This function tests deletion on a dictionary that has one element,
		but not the one we are looking for. We expect to receive err_item_not_found
		and for everything to remain as-is.
*/
void
test_cpp_wrapper_delete_nonexist_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 5, 10, boolean_true);
	cpp_wrapper_delete(tc, dict, 3, err_item_not_found, 0, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests deletion on a dictionary that has one element, but not
		the one we are looking for on all dictionary implementations. We expect to
		receive err_item_not_found and for everything else to remain as-is.
*/
void
test_cpp_wrapper_delete_nonexist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_nonexist_single(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
}

/**
@brief	This function tests deletion on a dictionary that has many elements,
		but not the one we are looking for. We expect to receive err_item_not_found
		and for everything to remain as-is.
*/
void
test_cpp_wrapper_delete_nonexist_several(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_medium);
	cpp_wrapper_delete(tc, dict, -100, err_item_not_found, 0, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests deletion on a dictionary that has many elements, but not
		the one we are looking for on all dictionary implementations. We expect to
		receive err_item_not_found and for everything else to remain as-is.
*/
void
test_cpp_wrapper_delete_nonexist_several_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_nonexist_several(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
}

/**
@brief	This function tests deletion on a dictionary that has a single element,
		which is the one we're looking for.
*/
void
test_cpp_wrapper_delete_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 3, 6, boolean_true);
	cpp_wrapper_delete(tc, dict, 3, err_ok, 1, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests deletion on a dictionary that has a single element,
		which is the one we are looking for on all dictionary implementations.
*/
void
test_cpp_wrapper_delete_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_single(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_single(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_single(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single(tc, dict);
}

/**
@brief	This function tests deletion on a dictionary that has many elements,
		of which contains the one we choose to delete.
*/
void
test_cpp_wrapper_delete_single_several(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_high);
	cpp_wrapper_delete(tc, dict, 700, err_ok, 1, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests deletion on a dictionary that has many elements, of
		which contains the one we choose to delete on all dictionary implementations.
*/
void
test_cpp_wrapper_delete_single_several_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_single_several(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_single_several(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_single_several(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single_several(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single_several(tc, dict);
}

/**
@brief	This function tests deletion of everything within a dictionary.
*/
void
test_cpp_wrapper_delete_all(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_edge_cases);

	int i;

	ION_FILL_LOW_LOOP(i) {
		cpp_wrapper_delete(tc, dict, i, err_ok, 1, boolean_true);
	}
	ION_FILL_MEDIUM_LOOP(i) {
		cpp_wrapper_delete(tc, dict, i, err_ok, 1, boolean_true);
	}
	ION_FILL_HIGH_LOOP(i) {
		cpp_wrapper_delete(tc, dict, i, err_ok, 1, boolean_true);
	}
	ION_FILL_EDGE_LOOP(i) {
		cpp_wrapper_delete(tc, dict, i, err_ok, 1, boolean_true);
	}
}

/**
@brief	This function tests deletion of everything within a dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_delete_all_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_all(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_all(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_all(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_all(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_all(tc, dict);
}

/**
@brief	This function tests an update on a dictionary that is empty. We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_empty_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_update(tc, dict, 3, 5, err_ok, 1, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests an update on a dictionary that is empty on all dictionary
		implementations. We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_empty_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_empty_single(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_empty_single(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_empty_single(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_empty_single(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_empty_single(tc, dict);
}

/**
@brief	This function tests an update on a dictionary that has one element,
		but not the one we are looking for. We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_nonexist_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 10, 4, boolean_true);
	cpp_wrapper_update(tc, dict, 3, 5, err_ok, 1, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests an update on a dictionary that has one element,
		but not the one we are looking for on all dictionary implementations.
		We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_nonexist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_nonexist_single(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_nonexist_single(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_nonexist_single(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_single(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_single(tc, dict);
}

/**
@brief	This function tests an update on a dictionary that has many elements,
		but not the one we are looking for. We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_nonexist_in_many(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_medium);
	cpp_wrapper_update(tc, dict, 63, -10, err_ok, 1, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests an update on a dictionary that has many elements,
		but not the one we are looking for on all dictionary implementations.
		We expect an upsert to occur.
*/
void
test_cpp_wrapper_update_nonexist_in_many_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
}

/**
@brief	This function tests an update on a dictionary that has one element,
		which is the one we are looking for.
*/
void
test_cpp_wrapper_update_exist_single(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
	cpp_wrapper_insert(tc, dict, 23, 0, boolean_true);
	cpp_wrapper_update(tc, dict, 23, 44, err_ok, 1, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests an update on a dictionary that has one element,
		which is the one we are looking for on all dictionary implementations.
*/
void
test_cpp_wrapper_update_exist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_exist_single(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_exist_single(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_exist_single(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_single(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_single(tc, dict);
}

/**
@brief	This function tests an update on a dictionary that has many elements,
		which includes the one we are looking for.
*/
void
test_cpp_wrapper_update_exist_in_many(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_medium);
	cpp_wrapper_update(tc, dict, 60, -23, err_ok, 1, boolean_true);
	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests an update on a dictionary that has many elements,
		which includes the one we are looking for on all dictionary implementations.
*/
void
test_cpp_wrapper_update_exist_in_many_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_exist_in_many(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_exist_in_many(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_exist_in_many(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_in_many(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_in_many(tc, dict);
}

/**
@brief	This function tests update of everything within a dictionary.
*/
void
test_cpp_wrapper_update_all(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_edge_cases);

	int i;

	ION_FILL_LOW_LOOP(i) {
		cpp_wrapper_update(tc, dict, i, -1337, err_ok, 1, boolean_true);
	}
	ION_FILL_MEDIUM_LOOP(i) {
		cpp_wrapper_update(tc, dict, i, -1337, err_ok, 1, boolean_true);
	}
	ION_FILL_HIGH_LOOP(i) {
		cpp_wrapper_update(tc, dict, i, -1337, err_ok, 1, boolean_true);
	}
	ION_FILL_EDGE_LOOP(i) {
		cpp_wrapper_update(tc, dict, i, -1337, err_ok, 1, boolean_true);
	}

	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests an update of everything within a dictionary on all
		dictionary implementations.
*/
void
test_cpp_wrapper_update_all_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_all(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_all(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_all(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_all(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_all(tc, dict);
}

/**
@brief	This function tests some deletes, followed by an insert. Nothing should go wrong.
*/
void
test_cpp_wrapper_delete_then_insert(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_edge_cases);

	cpp_wrapper_delete(tc, dict, 60, err_ok, 1, boolean_true);
	cpp_wrapper_delete(tc, dict, 4, err_ok, 1, boolean_true);
	cpp_wrapper_delete(tc, dict, 505, err_ok, 1, boolean_true);

	cpp_wrapper_insert(tc, dict, 61, 44, boolean_true);
	cpp_wrapper_insert(tc, dict, 67, 42, boolean_true);
	cpp_wrapper_insert(tc, dict, 73, 48, boolean_true);

	cpp_wrapper_destroy(tc, dict);
}

/**
@brief	This function tests some deletes, followed by an insert on all dictionary
		implementations. Nothing should go wrong.
*/
void
test_cpp_wrapper_delete_then_insert_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_then_insert(tc, dict);

	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_then_insert(tc, dict);

	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_then_insert(tc, dict);

	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_then_insert(tc, dict);

	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_then_insert(tc, dict);
}

/**
@brief	Tests an insertion and then attempts to perform an equality cursor query
		on implementations that support duplicate keys.
*/
void
test_cpp_wrapper_equality_duplicates(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int eq_key
) {
	int nums[eq_key];
	int records_found	= 0;
	int curr_pos		= 0;

	for (int i = 1; i <= eq_key; i++) {
		dict->insert(eq_key, i * 2);
		nums[i - 1] = eq_key;
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

		dict->insert(i - 1, (i - 1) * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *eq_cursor = dict->equality(eq_key);

	PLANCK_UNIT_ASSERT_TRUE(tc, eq_cursor->hasNext());

	ion_cursor_status_t status = eq_cursor->next();

	while (status) {
		for (int i = 0; i < eq_key; i++) {
			if (nums[i] == eq_cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], eq_cursor->getKey());

		nums[curr_pos]	= -1;
		status			= eq_cursor->next();
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, eq_cursor->hasNext());

	for (int i = 0; i < eq_key; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, nums[i]);
	}

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, eq_key, records_found);
	delete eq_cursor;
}

/**
@brief	Tests an insertion and then attempts to perform an equality cursor query
		on implementations that do not allow duplicate keys.
*/
void
test_cpp_wrapper_equality_no_duplicates(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int eq_key
) {
	int records_found = 0;

	for (int i = 0; i < eq_key + 3; i++) {
		dict->insert(i, i * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *eq_cursor = dict->equality(eq_key);

	PLANCK_UNIT_ASSERT_TRUE(tc, eq_cursor->hasNext());

	ion_cursor_status_t status = eq_cursor->next();

	while (status) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, eq_key, eq_cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, eq_key * 2, eq_cursor->getValue());
		status = eq_cursor->next();
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, eq_cursor->hasNext());

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, records_found);
	delete eq_cursor;
}

/**
@brief	Tests an insertion and then attempts to perform an equality cursor query
		when the desired key is not present.
*/
void
test_cpp_wrapper_equality_edge_case1(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	for (int i = 0; i < 10; i++) {
		dict->insert(i, i * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *eq_cursor = dict->equality(20);

	PLANCK_UNIT_ASSERT_FALSE(tc, eq_cursor->hasNext());
	PLANCK_UNIT_ASSERT_FALSE(tc, eq_cursor->next());

	delete eq_cursor;
}

/**
@brief	Aggregate test to test equality cursor query on all implementations.
*/
void
test_cpp_wrapper_equality_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_equality_duplicates(tc, dict, 10);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_equality_duplicates(tc, dict, 4);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_equality_no_duplicates(tc, dict, 4);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_no_duplicates(tc, dict, 3);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_no_duplicates(tc, dict, 6);
	delete dict;
}

/**
@brief	Aggregate test to test equality edge case cursor query on all implementations.
*/
void
test_cpp_wrapper_equality_edge_case1_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_equality_edge_case1(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_equality_edge_case1(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_equality_edge_case1(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_edge_case1(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_edge_case1(tc, dict);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to perform a simple equality cursor query
		for any desired range of keys.
*/
void
test_cpp_wrapper_range_simple(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int min_key,
	int max_key
) {
	/* records_expected is calculated the following way because it is min_key and max_key inclusive. */
	int records_expected	= max_key - min_key + 1;
	int records_found		= 0;
	int curr_pos			= 0;
	int nums[records_expected];

	PLANCK_UNIT_ASSERT_TRUE(tc, min_key < max_key);

	for (int i = 0; i < max_key + 5; i++) {
		dict->insert(i, i);

		if ((i >= min_key) && (i <= max_key)) {
			nums[curr_pos] = i;
			curr_pos++;
		}

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *range_cursor	= dict->range(min_key, max_key);
	PLANCK_UNIT_ASSERT_TRUE(tc, range_cursor->hasNext());

	ion_cursor_status_t status		= range_cursor->next();

	while (status) {
		for (int i = 0; i < max_key + 5; i++) {
			if (nums[i] == range_cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], range_cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], range_cursor->getValue());

		nums[curr_pos]	= -1;
		status			= range_cursor->next();
		min_key++;
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, range_cursor->hasNext());

	for (int i = 0; i < records_expected; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, nums[i]);
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, records_expected, records_found);
	delete range_cursor;
}

/**
@brief	Tests an insertion and then attempts to perform an edge case equality cursor query,
		i.e., range values are not present in dictionary.
*/
void
test_cpp_wrapper_range_edge_case1(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int total_records	= 0;
	int nums[5]			= { 3, 10, 25, 50, 70 };
	int nums_length		= sizeof(nums) / sizeof(int);
	int curr_pos		= 0;

	for (int i = 0; i < nums_length; i++) {
		dict->insert(nums[i], nums[i]);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *range_cursor = dict->range(-1000, 1000);

	PLANCK_UNIT_ASSERT_TRUE(tc, range_cursor->hasNext());

	ion_cursor_status_t status = range_cursor->next();

	while (status) {
		for (int i = 0; i < nums_length; i++) {
			if (nums[i] == range_cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], range_cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], range_cursor->getValue());
		nums[curr_pos]	= -1;
		status			= range_cursor->next();
		total_records++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, range_cursor->hasNext());

	for (int i = 0; i < nums_length; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, nums[i]);
	}

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums_length, total_records);
	delete range_cursor;
}

/**
@brief	Tests an insertion and then attempts to perform an edge case equality cursor query,
		i.e., desired records are in the middle of the dictionary.
*/
void
test_cpp_wrapper_range_edge_case2(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int total_records		= 0;
	int nums[5]				= { 3, 10, 25, 50, 70 };
	int expected_nums[2]	= { 25, 50 };
	int nums_length			= sizeof(nums) / sizeof(int);
	int curr_pos			= 0;

	for (int i = 0; i < nums_length; i++) {
		dict->insert(nums[i], nums[i]);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *range_cursor = dict->range(15, 55);

	PLANCK_UNIT_ASSERT_TRUE(tc, range_cursor->hasNext());

	ion_cursor_status_t status = range_cursor->next();

	while (status) {
		for (int i = 0; i < 2; i++) {
			if (expected_nums[i] == range_cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_nums[curr_pos], range_cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_nums[curr_pos], range_cursor->getValue());
		expected_nums[curr_pos] = -1;
		status					= range_cursor->next();
		total_records++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, range_cursor->hasNext());

	for (int i = 0; i < 2; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, expected_nums[i]);
	}

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, total_records);
	delete range_cursor;
}

/**
@brief	Tests an insertion and then attempts to perform an edge case equality cursor query,
		i.e., range query includes all records present in the dictionary.
*/
void
test_cpp_wrapper_range_edge_case3(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int total_records	= 0;
	int nums[5]			= { 1, 2, 3, 4, 5 };
	int nums_length		= sizeof(nums) / sizeof(int);
	int curr_pos		= 0;

	for (int i = 0; i < nums_length; i++) {
		dict->insert(nums[i], nums[i]);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *range_cursor = dict->range(1, 5);

	PLANCK_UNIT_ASSERT_TRUE(tc, range_cursor->hasNext());

	ion_cursor_status_t status = range_cursor->next();

	while (status) {
		for (int i = 0; i < nums_length; i++) {
			if (nums[i] == range_cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], range_cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], range_cursor->getValue());
		nums[curr_pos]	= -1;
		status			= range_cursor->next();
		total_records++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, range_cursor->hasNext());

	for (int i = 0; i < nums_length; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, nums[i]);
	}

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums_length, total_records);
	delete range_cursor;
}

/**
@brief	Aggregate test to test simple range cursor query on all implementations.
*/
void
test_cpp_wrapper_range_simple_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_range_simple(tc, dict, 5, 10);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_range_simple(tc, dict, 1, 3);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_range_simple(tc, dict, 4, 18);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_range_simple(tc, dict, 30, 36);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	test_cpp_wrapper_range_simple(tc, dict, 5, 7);
	delete dict;
}

/**
@brief	Aggregate test to test edge case 1 range cursor query on all implementations.
*/
void
test_cpp_wrapper_range_edge_case1_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_range_edge_case1(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_range_edge_case1(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_range_edge_case1(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_range_edge_case1(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	test_cpp_wrapper_range_edge_case1(tc, dict);
	delete dict;
}

/**
@brief	Aggregate test to test edge case 2 range cursor query on all implementations.
*/
void
test_cpp_wrapper_range_edge_case2_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_range_edge_case2(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_range_edge_case2(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_range_edge_case2(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_range_edge_case2(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	test_cpp_wrapper_range_edge_case2(tc, dict);
	delete dict;
}

/**
@brief	Aggregate test to test edge case 3 range cursor query on all implementations.
*/
void
test_cpp_wrapper_range_edge_case3_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_range_edge_case3(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_range_edge_case3(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_range_edge_case3(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_range_edge_case3(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	test_cpp_wrapper_range_edge_case3(tc, dict);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to perform a simple all records cursor query.
*/
void
test_cpp_wrapper_all_records_simple(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int random_positive_num
) {
	int total_records	= 0;
	int nums[random_positive_num];
	int curr_pos		= 0;

	for (int i = 0; i < random_positive_num; i++) {
		dict->insert(i, i * 2);
		nums[i] = i;
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *all_rec_cursor = dict->allRecords();

	PLANCK_UNIT_ASSERT_TRUE(tc, all_rec_cursor->hasNext());

	ion_cursor_status_t status = all_rec_cursor->next();

	while (status) {
		for (int i = 0; i < random_positive_num; i++) {
			if (nums[i] == all_rec_cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], all_rec_cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos] * 2, all_rec_cursor->getValue());
		nums[curr_pos]	= -1;

		total_records++;
		status			= all_rec_cursor->next();
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, all_rec_cursor->hasNext());

	for (int i = 0; i < random_positive_num; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, nums[i]);
	}

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, random_positive_num, total_records);
	delete all_rec_cursor;
}

/**
@brief	Tests the edge cases of an all records cursor query,
		i.e., keys are sparsely inserted.
*/
void
test_cpp_wrapper_all_records_edge_cases1(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int total_records	= 0;
	int nums[5]			= { 3, 10, 25, 50, 70 };
	int nums_length		= sizeof(nums) / sizeof(int);
	int curr_pos		= 0;

	for (int i = 0; i < nums_length; i++) {
		dict->insert(nums[i], nums[i]);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *all_rec_cursor = dict->allRecords();

	PLANCK_UNIT_ASSERT_TRUE(tc, all_rec_cursor->hasNext());

	ion_cursor_status_t status = all_rec_cursor->next();

	while (status) {
		for (int i = 0; i < nums_length; i++) {
			if (nums[i] == all_rec_cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], all_rec_cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], all_rec_cursor->getValue());
		nums[curr_pos]	= -1;
		status			= all_rec_cursor->next();
		total_records++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, all_rec_cursor->hasNext());

	for (int i = 0; i < nums_length; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, nums[i]);
	}

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums_length, total_records);
	delete all_rec_cursor;
}

/**
@brief	Tests the edge cases of an all records cursor query, i.e., keys
		are not inserted in increasing order and include a wide range.
*/
void
test_cpp_wrapper_all_records_edge_cases2(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int total_records	= 0;
	int nums[5]			= { -111, 113, 5, 50, -225 };
	int nums_length		= sizeof(nums) / sizeof(int);
	int curr_pos		= 0;

	for (int i = 0; i < nums_length; i++) {
		dict->insert(nums[i], nums[i]);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *all_rec_cursor = dict->allRecords();

	PLANCK_UNIT_ASSERT_TRUE(tc, all_rec_cursor->hasNext());

	ion_cursor_status_t status = all_rec_cursor->next();

	while (status) {
		for (int i = 0; i < nums_length; i++) {
			if (nums[i] == all_rec_cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], all_rec_cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums[curr_pos], all_rec_cursor->getValue());
		nums[curr_pos]	= -1;
		status			= all_rec_cursor->next();
		total_records++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, all_rec_cursor->hasNext());

	for (int i = 0; i < nums_length; i++) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, -1, nums[i]);
	}

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, nums_length, total_records);
	delete all_rec_cursor;
}

/**
@brief	Aggregate test to test a simple all records cursor query on all implementations.
*/
void
test_cpp_wrapper_all_records_simple_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_all_records_simple(tc, dict, 10);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_all_records_simple(tc, dict, 4);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_all_records_simple(tc, dict, 13);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_all_records_simple(tc, dict, 5);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_all_records_simple(tc, dict, 8);
	delete dict;
}

/**
@brief	Aggregate test to test all records edge case 1 cursor query on all implementations.
*/
void
test_cpp_wrapper_all_records_edge_cases1_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_all_records_edge_cases1(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_all_records_edge_cases1(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_all_records_edge_cases1(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_all_records_edge_cases1(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_all_records_edge_cases1(tc, dict);
	delete dict;
}

/**
@brief	Aggregate test to test all records edge case 2 cursor query on all implementations.
*/
void
test_cpp_wrapper_all_records_edge_cases2_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_all_records_edge_cases2(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_all_records_edge_cases2(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_all_records_edge_cases2(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_all_records_edge_cases2(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_all_records_edge_cases2(tc, dict);
	delete dict;
}

/**
@brief	Tests open and close functionality of a dictionary.
*/
void
test_cpp_wrapper_open_close(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int key,
	int value
) {
	ion_status_t			status;
	ion_err_t				error;
	ion_dictionary_id_t		gdict_id	= dict->dict.instance->id;
	ion_key_size_t			key_size	= dict->dict.instance->record.key_size;
	ion_value_size_t		val_size	= dict->dict.instance->record.value_size;
	ion_key_type_t			key_type	= dict->dict.instance->key_type;
	ion_dictionary_size_t	dict_size	= dict->dict_size;

	/* Insert test record so we can check data integrity after we close/open */
	status = dict->insert(key, value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	/* Check the test record */
	int ret_val = dict->get(key);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value, ret_val);

	error = dict->close();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	ion_dictionary_config_info_t config = {
		gdict_id, 0, key_type, key_size, val_size, dict_size
	};

	error = dict->open(config);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dict->dict.instance->record.key_size, key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dict->dict.instance->record.value_size, val_size);

	/* Check the test record */
	ret_val = dict->get(key);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value, ret_val);
}

/**
@brief	Aggregate test to test open/close functionality on all implementations.
*/
void
test_cpp_wrapper_open_close_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_open_close(tc, dict, 66, 12);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	test_cpp_wrapper_open_close(tc, dict, 45, 14);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_open_close(tc, dict, 3, 15);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_open_close(tc, dict, 5, 12);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_open_close(tc, dict, 1, 13);
	delete dict;
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper_getsuite_1(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_create);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_setup_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_insert_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_insert_multiple_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_in_many_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_lots_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_nonexist_empty_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_nonexist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_nonexist_many_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_exist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_populated_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_populated_multiple_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_all_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_empty_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_nonexist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_nonexist_several_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_single_several_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_all_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_empty_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_nonexist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_nonexist_in_many_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_exist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_exist_in_many_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_all_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_then_insert_all);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_equality_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_equality_edge_case1_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_range_simple_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_range_edge_case1_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_range_edge_case2_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_range_edge_case3_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_all_records_simple_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_all_records_edge_cases1_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_all_records_edge_cases2_on_all_implementations);

	return suite;
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper_getsuite_2(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_open_close_on_all_implementations);

	return suite;
}

/**
@brief	  Runs all C++ Related related tests and outputs the result.
*/
void
runalltests_cpp_wrapper(
) {
	planck_unit_suite_t *suite1 = cpp_wrapper_getsuite_1();

	planck_unit_run_suite(suite1);
	planck_unit_destroy_suite(suite1);

	planck_unit_suite_t *suite2 = cpp_wrapper_getsuite_2();

	planck_unit_run_suite(suite2);
	planck_unit_destroy_suite(suite2);
}
