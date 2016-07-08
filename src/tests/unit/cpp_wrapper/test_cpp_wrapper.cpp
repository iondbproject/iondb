/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Unit test for the C++ Wrapper
@todo		These tests need to be fully fleshed out.
*/
/******************************************************************************/

#include "../../planckunit/src/planck_unit.h"
#include "../../../cpp_wrapper/Dictionary.h"
#include "../../../cpp_wrapper/BppTree.h"
#include "../../../cpp_wrapper/FlatFile.h"
/* #include "../../../cpp_wrapper/OpenAddressFileHash.h" */
#include "../../../cpp_wrapper/OpenAddressHash.h"
#include "../../../cpp_wrapper/SkipList.h"
#include "../../../kv_system.h"

/**
@brief	Tests the creation of a B+ Tree (arbitrarily chosen) dictionary and asserts
		the validity of some dictionary parameters.
*/
void
test_cpp_wrapper_create_and_destroy(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));

	PLANCK_UNIT_ASSERT_TRUE(tc, dict->dict.instance->key_type == key_type_numeric_signed);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict->dict.instance->compare == dictionary_compare_signed_value);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict->dict.instance->record.key_size == sizeof(int));
	PLANCK_UNIT_ASSERT_TRUE(tc, dict->dict.instance->record.value_size == sizeof(int));

	delete dict;
}

/**
@brief	Tests an insertion and then attempts to retrieve.
*/
void
test_cpp_wrapper_insert_get(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	for (int i = 0; i < 10; i++) {
		dict->insert(i, i * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

		int ret_val = dict->get(i);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, i * 2, ret_val);
	}
}

/**
@brief	Tests insert/get edge cases such as negative keys and big-digit keys.
*/
void
test_cpp_wrapper_insert_get_edge_cases(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int ret_val;

	dict->insert(-10, 3);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

	ret_val = dict->get(-10);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 3, ret_val);

	dict->insert(1000, 99);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

	ret_val = dict->get(1000);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 99, ret_val);
}

/**
@brief	Aggregate test to test insertion on all implementations.
*/
void
test_cpp_wrapper_insert_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_insert_get(tc, dict);
	test_cpp_wrapper_insert_get_edge_cases(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_insert_get(tc, dict);
	test_cpp_wrapper_insert_get_edge_cases(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_insert_get(tc, dict);
	test_cpp_wrapper_insert_get_edge_cases(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_get(tc, dict);
	test_cpp_wrapper_insert_get_edge_cases(tc, dict);
	delete dict;
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_create_and_destroy);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_insert_on_all_implementations);

	return suite;
}

/**
@brief	  Runs all C++ Related related tests and outputs the result.
 */
void
runalltests_cpp_wrapper(
) {
	planck_unit_suite_t *suite = cpp_wrapper_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
