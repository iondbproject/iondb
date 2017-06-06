/******************************************************************************/
/**
@file		test_cpp_wrapper.cpp
@author		Kris Wallperington, Dana Klamut
@brief		Unit test for the C++ Wrapper
@todo		These tests need to be fully fleshed out.
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without 
	modification, are permitted provided that the following conditions are met:
	
@par 1.Redistributions of source code must retain the above copyright notice, 
	this list of conditions and the following disclaimer.
	
@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation 
	and/or other materials provided with the distribution.
	
@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission. 
	
@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#include "../../planckunit/src/planck_unit.h"
#include "../../../cpp_wrapper/Dictionary.h"
#include "../../../cpp_wrapper/BppTree.h"
#include "../../../cpp_wrapper/FlatFile.h"
#include "../../../cpp_wrapper/OpenAddressFileHash.h"
#include "../../../cpp_wrapper/OpenAddressHash.h"
#include "../../../cpp_wrapper/SkipList.h"
#include "test_cpp_wrapper.h"

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

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_insert_get(tc, dict);
	test_cpp_wrapper_insert_get_edge_cases(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_get(tc, dict);
	test_cpp_wrapper_insert_get_edge_cases(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_get(tc, dict);
	test_cpp_wrapper_insert_get_edge_cases(tc, dict);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to delete the record.
*/
void
test_cpp_wrapper_insert_delete(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	for (int i = 0; i < 10; i++) {
		dict->insert(i, i * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

		dict->deleteRecord(i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

		dict->get(i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, dict->last_status.count);
	}
}

/**
@brief	Tests insert/delete edge cases such as negative keys and big-digit keys.
*/
void
test_cpp_wrapper_insert_delete_edge_cases(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	dict->insert(-10, 3);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

	dict->deleteRecord(-10);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

	dict->get(-10);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, dict->last_status.count);

	dict->insert(1000, 99);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

	dict->deleteRecord(1000);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

	dict->get(1000);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, dict->last_status.count);
}

/**
@brief	Aggregate test to test deletion on all implementations.
*/
void
test_cpp_wrapper_delete_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_insert_delete(tc, dict);
	test_cpp_wrapper_insert_delete_edge_cases(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_insert_delete(tc, dict);
	test_cpp_wrapper_insert_delete_edge_cases(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_insert_delete(tc, dict);
	test_cpp_wrapper_insert_delete_edge_cases(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_delete(tc, dict);
	test_cpp_wrapper_insert_delete_edge_cases(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_delete(tc, dict);
	test_cpp_wrapper_insert_delete_edge_cases(tc, dict);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to update the record.
*/
void
test_cpp_wrapper_insert_update(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	for (int i = 0; i < 10; i++) {
		dict->insert(i, i * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

		dict->update(i, i);

		int ret_val = dict->get(i);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, i, ret_val);
	}
}

/**
@brief	Tests insert/update edge cases such as negative keys and big-digit keys.
*/
void
test_cpp_wrapper_insert_update_edge_cases(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	int ret_val;

	dict->insert(-10, 3);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

	dict->update(-10, 30);
	ret_val = dict->get(-10);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 30, ret_val);

	dict->insert(1000, 99);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

	dict->update(1000, 9);
	ret_val = dict->get(1000);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 9, ret_val);
}

/**
@brief	Aggregate test to test update on all implementations.
*/
void
test_cpp_wrapper_update_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_insert_update(tc, dict);
	test_cpp_wrapper_insert_update_edge_cases(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_insert_update(tc, dict);
	test_cpp_wrapper_insert_update_edge_cases(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_insert_update(tc, dict);
	test_cpp_wrapper_insert_update_edge_cases(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_update(tc, dict);
	test_cpp_wrapper_insert_update_edge_cases(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_update(tc, dict);
	test_cpp_wrapper_insert_update_edge_cases(tc, dict);
	delete dict;
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
	dict	= new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_open_close(tc, dict, 66, 12);
	delete dict;
	dict	= new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	test_cpp_wrapper_open_close(tc, dict, 45, 14);
	delete dict;
	dict	= new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_open_close(tc, dict, 3, 15);
	delete dict;
	dict	= new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_open_close(tc, dict, 5, 12);
	delete dict;
	dict	= new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
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

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_create_and_destroy);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_insert_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_on_all_implementations);
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
