/******************************************************************************/
/**
@file		test_cpp_wrapper2.cpp
@author		Eric Huang, Dana Klamut
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

#include "test_cpp_wrapper2.h"

#define NULL_VALUE 999	/** Value arbitrarily chosen to represent NULL */

/**
@brief	This function opens a dictionary using the master table.
*/
void
master_table_open_dictionary(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary,
	ion_dictionary_id_t id
) {
	ion_err_t err = master_table->openDictionary(dictionary, id);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
}

/**
@brief	This function closes a dictionary using the master table.
*/
void
master_table_close_dictionary(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary
) {
	ion_err_t err = master_table->closeDictionary(dictionary);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_closed, dictionary->dict.status);
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
@brief	This function performs an equality cursor query on a dictionary.
*/
void
cpp_wrapper_equality(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int key,
	int expected_values[],
	int expected_num_records,
	ion_boolean_t key_exists
) {
	Cursor<int, int> *cursor = dict->equality(key);

	if (!key_exists) {
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->next());

		delete cursor;

		return;
	}

	int records_found	= 0;
	int curr_pos		= 0;

	PLANCK_UNIT_ASSERT_TRUE(tc, cursor->hasNext());

	ion_cursor_status_t status = cursor->next();

	while (status) {
		for (int i = 0; i < key; i++) {
			if (expected_values[i] == cursor->getValue()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key, cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_values[curr_pos], cursor->getValue());

		expected_values[curr_pos]	= NULL_VALUE;
		status						= cursor->next();
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_num_records, records_found);

	delete cursor;
}

/**
@brief	This function performs a range cursor query on a dictionary.
*/
void
cpp_wrapper_range(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int min_key,
	int max_key,
	int expected_records[],
	int expected_num_records,
	ion_boolean_t records_exist
) {
	PLANCK_UNIT_ASSERT_TRUE(tc, min_key < max_key);

	Cursor<int, int> *cursor = dict->range(min_key, max_key);

	if (!records_exist) {
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->next());

		delete cursor;

		return;
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cursor->hasNext());

	int records_found			= 0;
	int curr_pos				= 0;

	ion_cursor_status_t status	= cursor->next();

	while (status) {
		for (int i = 0; i < expected_num_records; i++) {
			if (expected_records[i] == cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_records[curr_pos], cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_records[curr_pos], cursor->getValue());

		expected_records[curr_pos]	= NULL_VALUE;
		status						= cursor->next();
		min_key++;
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_num_records, records_found);

	delete cursor;
}

/**
@brief	This function performs an all records cursor query on a dictionary.
*/
void
cpp_wrapper_all_records(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int expected_records[],
	int expected_num_records,
	ion_boolean_t records_exist
) {
	Cursor<int, int> *cursor = dict->allRecords();

	if (!records_exist) {
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());
		PLANCK_UNIT_ASSERT_FALSE(tc, cursor->next());

		delete cursor;

		return;
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cursor->hasNext());

	int records_found			= 0;
	int curr_pos				= 0;

	ion_cursor_status_t status	= cursor->next();

	while (status) {
		for (int i = 0; i < expected_num_records; i++) {
			if (expected_records[i] == cursor->getKey()) {
				curr_pos = i;
				break;
			}
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_records[curr_pos], cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_records[curr_pos], cursor->getValue());

		expected_records[curr_pos]	= NULL_VALUE;
		status						= cursor->next();
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, cursor->hasNext());

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_num_records, records_found);

	delete cursor;
}

/* =================================================== TEST CASES =================================================== */

/**
@brief	This function tests whether or not we can build a dictionary.
*/
void
test_cpp_wrapper_setup(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
) {
	cpp_wrapper_setup(tc, dict, ion_fill_none);
}

/**
@brief	This function tests whether or not we can build and a dictionary
		on all dictionary implementations.
*/
void
test_cpp_wrapper_setup_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_setup(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_setup(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_setup(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_setup(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_setup(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_setup(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_single(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_insert_multiple(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_single(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_in_many(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_lots(tc, dict);
	delete dict;
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
	cpp_wrapper_get(tc, dict, 99, NULL_VALUE, err_item_not_found, 0);
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_empty(tc, dict);
	delete dict;
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
	cpp_wrapper_get(tc, dict, 99, NULL_VALUE, err_item_not_found, 0);
}

/**
@brief	This function tests retrieval on a dictionary for a single key that
		doesn't exist on all dictionary implementations.
*/
void
test_cpp_wrapper_get_nonexist_single_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_single(tc, dict);
	delete dict;
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
	cpp_wrapper_get(tc, dict, -2000, NULL_VALUE, err_item_not_found, 0);
	cpp_wrapper_get(tc, dict, 3000, NULL_VALUE, err_item_not_found, 0);
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_nonexist_many(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_exist_single(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_single(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_populated_multiple(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_get_all(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_empty_single(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_single(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_nonexist_in_many(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_single(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_exist_in_many(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_update_all(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_all(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_update_all(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_update_all(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_update_all(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_update_all(tc, dict);
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
	int key
) {
	int values[key];

	for (int i = 1; i <= key; i++) {
		cpp_wrapper_insert(tc, dict, key, i * 2, boolean_false);
		values[i - 1] = i * 2;

		cpp_wrapper_insert(tc, dict, i - 1, (i - 1) * 2, boolean_false);
	}

	cpp_wrapper_equality(tc, dict, key, values, key, boolean_true);
}

/**
@brief	Tests an insertion and then attempts to perform an equality cursor query
		on implementations that do not allow duplicate keys.
*/
void
test_cpp_wrapper_equality_no_duplicates(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int key
) {
	for (int i = 0; i < key + 3; i++) {
		cpp_wrapper_insert(tc, dict, i, i * 2, boolean_false);
	}

	int values[1] = { key * 2 };

	cpp_wrapper_equality(tc, dict, key, values, 1, boolean_true);
}

/**
@brief	Aggregate test to test equality cursor query on all dictionary implementations.
*/
void
test_cpp_wrapper_equality(
	planck_unit_test_t	*tc,
	int					key
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_equality_duplicates(tc, dict, key);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_equality_duplicates(tc, dict, key);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_equality_no_duplicates(tc, dict, key);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_no_duplicates(tc, dict, key);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 20);
	test_cpp_wrapper_equality_no_duplicates(tc, dict, key);
	delete dict;
}

/**
@brief	Tests an insertion and then attempts to perform a range cursor query
		for any desired range of keys.
*/
void
test_cpp_wrapper_range(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int min_key,
	int max_key
) {
	/* records_expected is calculated the following way because it is min_key and max_key inclusive. */
	int records_expected	= max_key - min_key + 1;
	int curr_pos			= 0;
	int nums[records_expected];

	for (int i = 0; i < max_key + 5; i++) {
		cpp_wrapper_insert(tc, dict, i, i, boolean_true);

		if ((i >= min_key) && (i <= max_key)) {
			nums[curr_pos] = i;
			curr_pos++;
		}
	}

	cpp_wrapper_range(tc, dict, min_key, max_key, nums, records_expected, boolean_true);
}

/**
@brief	Tests an insertion and then attempts to perform an all records cursor query.
*/
void
test_cpp_wrapper_all_records(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int random_positive_num
) {
	int records[random_positive_num];

	for (int i = 0; i < random_positive_num; i++) {
		cpp_wrapper_insert(tc, dict, i, i, boolean_true);
		records[i] = i;
	}

	cpp_wrapper_all_records(tc, dict, records, random_positive_num, boolean_true);
}

/**
@brief	This function tests deleting a closed instance of the master table.
*/
void
test_master_table_open_close(
	planck_unit_test_t *tc
) {
	MasterTable *master_table = new MasterTable();

	master_table_setup(tc, master_table);
	master_table_init(tc, master_table);
	master_table_close(tc, master_table);
	delete master_table;
}

/**
@brief	This function tests opening and closing a dictionary using the master table.
*/
void
test_master_table_dictionary_open_close(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dictionary,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
) {
	MasterTable *master_table = new MasterTable();

	ion_dictionary_id_t id;

	master_table_setup(tc, master_table);
	master_table_init(tc, master_table);
	master_table_dictionary_add(tc, master_table, dictionary, key_type_numeric_signed, sizeof(int), sizeof(int), dictionary_size, dictionary_type);

	id = dictionary->dict.instance->id;

	master_table_close_dictionary(tc, master_table, dictionary);
	master_table_open_dictionary(tc, master_table, dictionary, id);

	delete dictionary;

	master_table_delete_from_master_table(tc, master_table, id);

	delete master_table;
}

/**
@brief Tests opening and closing a dictionary using the master table on all
		dictionary implementations.
*/
void
test_master_table_dictionary_open_close_all_1(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dictionary;

	dictionary = new BppTree<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_open_close(tc, dictionary, 0, dictionary_type_bpp_tree_t);

	dictionary = new FlatFile<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_open_close(tc, dictionary, 30, dictionary_type_flat_file_t);

	dictionary = new OpenAddressHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_open_close(tc, dictionary, 50, dictionary_type_open_address_hash_t);

	dictionary = new OpenAddressFileHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_open_close(tc, dictionary, 50, dictionary_type_open_address_file_hash_t);

	dictionary = new SkipList<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_open_close(tc, dictionary, 7, dictionary_type_skip_list_t);

	/* Uncomment when LinearHash dictionary open memory issue fixed. */
/*	dictionary = new LinearHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 7); */
/*	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status); */
/*	test_master_table_dictionary_open_close(tc, dictionary, 7, dictionary_type_linear_hash_t); */
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper2_getsuite_1(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_setup_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_insert_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_insert_multiple_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_in_many_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_lots_all);

	return suite;
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper2_getsuite_2(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_nonexist_empty_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_nonexist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_nonexist_many_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_exist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_populated_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_populated_multiple_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_get_all_all);

	return suite;
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper2_getsuite_3(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_empty_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_nonexist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_nonexist_in_many_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_exist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_exist_in_many_all);

	return suite;
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper2_getsuite_4(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_all_all);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_master_table_open_close);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_master_table_dictionary_open_close_all_1);

	return suite;
}

/**
@brief	  Runs all C++ Related related tests and outputs the result.
*/
void
runalltests_cpp_wrapper2(
) {
	fdeleteall();

	planck_unit_suite_t *suite1 = cpp_wrapper2_getsuite_1();

	planck_unit_run_suite(suite1);
	planck_unit_destroy_suite(suite1);

	planck_unit_suite_t *suite2 = cpp_wrapper2_getsuite_2();

	planck_unit_run_suite(suite2);
	planck_unit_destroy_suite(suite2);

	planck_unit_suite_t *suite3 = cpp_wrapper2_getsuite_3();

	planck_unit_run_suite(suite3);
	planck_unit_destroy_suite(suite3);

	planck_unit_suite_t *suite4 = cpp_wrapper2_getsuite_4();

	planck_unit_run_suite(suite4);
	planck_unit_destroy_suite(suite4);
}
