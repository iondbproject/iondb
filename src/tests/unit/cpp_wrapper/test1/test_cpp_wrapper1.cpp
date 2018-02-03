/******************************************************************************/
/**
@file		test_cpp_wrapper1.cpp
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

#include "test_cpp_wrapper1.h"

#define NULL_VALUE 999	/** Value arbitrarily chosen to represent NULL */

/**
@brief	This function initializes the master table.
*/
void
master_table_init(
	planck_unit_test_t	*tc,
	MasterTable			*master_table
) {
	ion_err_t err = master_table->initializeMasterTable();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != ion_master_table_file);
}

/**
@brief	This function closes the master table.
*/
void
master_table_close(
	planck_unit_test_t	*tc,
	MasterTable			*master_table
) {
	ion_err_t err = master_table->closeMasterTable();

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == ion_master_table_file);
}

/**
@brief	This function performs cleanup if the master table was not previously
		destroyed properly.
*/
void
master_table_setup(
	planck_unit_test_t	*tc,
	MasterTable			*master_table
) {
	master_table_close(tc, master_table);
	fremove(ION_MASTER_TABLE_FILENAME);
}

/**
@brief	This function performs the dictionary initialization using the master table.
*/
void
master_table_dictionary_add(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary,
	ion_key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
) {
	ion_err_t err = master_table->addToMasterTable(dictionary, dictionary_size);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_type, dictionary->dict.instance->key_type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_size, dictionary->dict.instance->record.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value_size, dictionary->dict.instance->record.value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type, dictionary->dict.instance->type);
}

/**
@brief	Deletes the tracking of a dictionary from the master table.
*/
void
master_table_delete_from_master_table(
	planck_unit_test_t	*tc,
	MasterTable			*master_table,
	ion_dictionary_id_t id
) {
	ion_err_t err = master_table->deleteFromMasterTable(id);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

	ion_dictionary_config_info_t config;

	err = master_table->lookupMasterTable(id, &config);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_item_not_found, err);
}

/**
@brief	This function looks up a dictionary in the master table using the
		dictionary's ID and configurations.
*/
void
master_table_lookup_dictionary(
	planck_unit_test_t				*tc,
	MasterTable						*master_table,
	ion_dictionary_id_t				id,
	ion_key_type_t					key_type,
	ion_key_size_t					key_size,
	ion_value_size_t				value_size,
	ion_dictionary_size_t			dictionary_size,
	ion_dictionary_type_t			dictionary_type,
	ion_dictionary_config_info_t	*config,
	ion_boolean_t					expect_found
) {
	ion_err_t err = master_table->lookupMasterTable(id, config);

	if (expect_found) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, id, config->id);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_type, config->type);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_size, config->key_size);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value_size, config->value_size);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_size, config->dictionary_size);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type, config->dictionary_type);
	}
	else {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_item_not_found, err);
	}
}

/**
@brief	This function creates a dictionary using the master table.
*/
void
master_table_create_dictionary(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary,
	ion_dictionary_id_t id,
	ion_key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
) {
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, id, dictionary->dict.instance->id);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_type, dictionary->dict.instance->key_type);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, key_size, dictionary->dict.instance->record.key_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, value_size, dictionary->dict.instance->record.value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_size, dictionary->dict_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_type, dictionary->dict.instance->type);

	ion_dictionary_config_info_t config;

	master_table_lookup_dictionary(tc, master_table, id, key_type, key_size, value_size, dictionary_size, dictionary_type, &config, boolean_true);
}

/**
@brief	This function deletes a dictionary using the master table.
*/
void
master_table_delete_dictionary(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary
) {
	ion_dictionary_id_t id	= dictionary->dict.instance->id;

	ion_err_t err			= master_table->deleteDictionary(dictionary);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);

	ion_dictionary_config_info_t config;

	err = master_table->lookupMasterTable(id, &config);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_item_not_found, err);
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
	int				retval	= dict->get(key);
	ion_status_t	status	= dict->last_status;

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
		cpp_wrapper_get(tc, dict, key, NULL_VALUE, err_item_not_found, 0);
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_empty(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_single(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_nonexist_several(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_single_several(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;
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

	dict = new BppTree<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;

	dict = new SkipList<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;

	dict = new FlatFile<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;

	dict = new OpenAddressHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;

	dict = new LinearHash<int, int>(0, key_type_numeric_signed, sizeof(int), sizeof(int), 160);
	test_cpp_wrapper_delete_then_insert(tc, dict);
	delete dict;
}

/**
@brief	This function tests deleting an open instance of the master table.
*/
void
test_master_table_delete(
	planck_unit_test_t *tc
) {
	MasterTable *master_table = new MasterTable();

	master_table_setup(tc, master_table);
	master_table_init(tc, master_table);
	delete master_table;
}

/**
@brief	This function tests creating and deleting a dictionary using the master table.
*/
void
test_master_table_dictionary_create_delete(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dictionary,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
) {
	MasterTable *master_table = new MasterTable();

	master_table_setup(tc, master_table);
	master_table_init(tc, master_table);
	master_table_dictionary_add(tc, master_table, dictionary, key_type_numeric_signed, sizeof(int), sizeof(int), dictionary_size, dictionary_type);

	ion_dictionary_id_t id = dictionary->dict.instance->id;

	delete dictionary;

	master_table_delete_from_master_table(tc, master_table, id);

	delete master_table;
}

/**
@brief Tests creating and deleting a dictionary using the master table on all
		dictionary implementations.
*/
void
test_master_table_dictionary_create_delete_all_1(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dictionary;

	dictionary = new BppTree<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 0, dictionary_type_bpp_tree_t);

	dictionary = new FlatFile<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 30);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 30, dictionary_type_flat_file_t);

	dictionary = new OpenAddressHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 50, dictionary_type_open_address_hash_t);

	dictionary = new OpenAddressFileHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 50, dictionary_type_open_address_file_hash_t);

	dictionary = new SkipList<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 7, dictionary_type_skip_list_t);

	dictionary = new LinearHash<int, int>(1, key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, ion_dictionary_status_ok, dictionary->dict.status);
	test_master_table_dictionary_create_delete(tc, dictionary, 7, dictionary_type_linear_hash_t);
}

/**
@brief	This function tests creating and deleting a dictionary using the master table.
*/
void
test_master_table_dictionary_create_delete(
	planck_unit_test_t		*tc,
	ion_dictionary_size_t	dictionary_size,
	ion_dictionary_type_t	dictionary_type
) {
	MasterTable *master_table = new MasterTable();

	master_table_setup(tc, master_table);
	master_table_init(tc, master_table);

	Dictionary<int, int> *dictionary;

	int type = 0;

	dictionary = master_table->initializeDictionary(key_type_numeric_signed, type, type, sizeof(int), sizeof(int), dictionary_size, dictionary_type);
	master_table_create_dictionary(tc, master_table, dictionary, 1, key_type_numeric_signed, sizeof(int), sizeof(int), dictionary_size, dictionary_type);
	master_table_delete_dictionary(tc, master_table, dictionary);

	delete master_table;
}

/**
@brief Tests creating and deleting a dictionary using the master table on all
		dictionary implementations created using the master table.
*/
void
test_master_table_dictionary_create_delete_all_2(
	planck_unit_test_t *tc
) {
	test_master_table_dictionary_create_delete(tc, 0, dictionary_type_bpp_tree_t);

	test_master_table_dictionary_create_delete(tc, 30, dictionary_type_flat_file_t);

	test_master_table_dictionary_create_delete(tc, 50, dictionary_type_open_address_hash_t);

	test_master_table_dictionary_create_delete(tc, 50, dictionary_type_open_address_file_hash_t);

	test_master_table_dictionary_create_delete(tc, 7, dictionary_type_skip_list_t);

	test_master_table_dictionary_create_delete(tc, 7, dictionary_type_linear_hash_t);
}

/**
@brief	This function tests deletion of everything within a dictionary on all
		dictionary implementations.
*/
void
test_static_delete_all_all(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	ion_dictionary_config_info_t config = {
		.id = 1, .use_type = 0, .type = key_type_numeric_signed, .key_size = sizeof(int), .value_size = sizeof(int), .dictionary_size = 100, .dictionary_type = dictionary_type_bpp_tree_t, .dictionary_status = err_ok
	};

	int type = 0;

	dict = BppTree<int, int>::openDictionary(config, type, type);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	config.dictionary_size	= 100;
	config.dictionary_type	= dictionary_type_skip_list_t;

	dict					= SkipList<int, int>::openDictionary(config, type, type);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	config.dictionary_size	= 100;
	config.dictionary_type	= dictionary_type_flat_file_t;

	dict					= FlatFile<int, int>::openDictionary(config, type, type);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	config.dictionary_size	= 160;
	config.dictionary_type	= dictionary_type_open_address_hash_t;

	dict					= OpenAddressHash<int, int>::openDictionary(config, type, type);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	config.dictionary_size	= 160;
	config.dictionary_type	= dictionary_type_open_address_file_hash_t;

	dict					= OpenAddressFileHash<int, int>::openDictionary(config, type, type);
	test_cpp_wrapper_delete_all(tc, dict);
	delete dict;

	/* Uncomment when LinearHash dictionary open memory issue fixed. */
/*	config.dictionary_size = 100; */
/*	config.dictionary_type = dictionary_type_linear_hash_t}; */
/*  */
/*	dict = LinearHash<int, int>::openDictionary(config, type, type); */
/*	test_cpp_wrapper_delete_all(tc, dict); */
/*	delete dict; */
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper1_getsuite_1(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_empty_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_nonexist_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_nonexist_several_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_single_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_single_several_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_all_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_then_insert_all);

	return suite;
}

/**
@brief		Creates the suite to test.
@return		Pointer to a test suite.
*/
planck_unit_suite_t *
cpp_wrapper1_getsuite_2(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_master_table_delete);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_master_table_dictionary_create_delete_all_1);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_master_table_dictionary_create_delete_all_2);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_static_delete_all_all);

	return suite;
}

/**
@brief	  Runs all C++ Related related tests and outputs the result.
*/
void
runalltests_cpp_wrapper1(
) {
	fdeleteall();

	planck_unit_suite_t *suite1 = cpp_wrapper1_getsuite_1();

	planck_unit_run_suite(suite1);
	planck_unit_destroy_suite(suite1);

	planck_unit_suite_t *suite2 = cpp_wrapper1_getsuite_2();

	planck_unit_run_suite(suite2);
	planck_unit_destroy_suite(suite2);
}
