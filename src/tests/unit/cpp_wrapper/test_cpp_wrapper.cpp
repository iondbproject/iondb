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
#include "../../../cpp_wrapper/OpenAddressFileHash.h"
#include "../../../cpp_wrapper/OpenAddressHash.h"
#include "../../../cpp_wrapper/SkipList.h"
#include "test_cpp_wrapper.h"
#include "../../../key_value/kv_system.h"

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

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
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

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
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
@brief	Tests an insertion and then attempts to perform an equality cursor query.
		Results are expected in increasing order.
*/
void
test_cpp_wrapper_equality_increasing(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int eq_key
) {
	int records_expected	= eq_key;
	int records_found		= 0;
	int expected_value		= eq_key;

	for (int i = 1; i <= eq_key; i++) {
		dict->insert(eq_key, i * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

		dict->insert(i - 1, (i - 1) * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *eq_cursor = dict->equality(eq_key);

	PLANCK_UNIT_ASSERT_TRUE(tc, eq_cursor->hasNext());

	cursor_status_t status = eq_cursor->next();

	while (status) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, eq_key, eq_cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_value * 2, eq_cursor->getValue());
		status = eq_cursor->next();
		expected_value--;
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, eq_cursor->hasNext());

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, records_expected, records_found);
	delete eq_cursor;
}

/**
@brief	Tests an insertion and then attempts to perform an equality cursor query.
		Results are expected in decreasing order.
*/
void
test_cpp_wrapper_equality_decreasing(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int eq_key
) {
	int records_expected	= eq_key;
	int records_found		= 0;

	/* This value starts at one as that is what our value insertion counter starts at. */
	int expected_value		= 1;

	for (int i = 1; i <= eq_key; i++) {
		dict->insert(eq_key, i * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);

		dict->insert(i - 1, (i - 1) * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *eq_cursor = dict->equality(eq_key);

	PLANCK_UNIT_ASSERT_TRUE(tc, eq_cursor->hasNext());

	cursor_status_t status = eq_cursor->next();

	while (status) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, eq_key, eq_cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_value * 2, eq_cursor->getValue());
		status = eq_cursor->next();
		expected_value++;
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, eq_cursor->hasNext());

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, records_expected, records_found);
	delete eq_cursor;
}

/**
@brief	Tests an insertion and then attempts to perform an equality cursor query
		on implementations that do not allow duplicate key insertions.
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

	cursor_status_t status = eq_cursor->next();

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
@brief	Aggregate test to test equality cursor query on all implementations.
*/
void
test_cpp_wrapper_equality_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_equality_increasing(tc, dict, 10);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_equality_decreasing(tc, dict, 4);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
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
@brief	Tests an insertion and then attempts to perform an equality cursor query.
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
	int records_found		= 0;

	PLANCK_UNIT_ASSERT_TRUE(tc, min_key < max_key);

	for (int i = 0; i < max_key + 5; i++) {
		dict->insert(i, i);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *range_cursor	= dict->range(min_key, max_key);
	PLANCK_UNIT_ASSERT_TRUE(tc, range_cursor->hasNext());

	cursor_status_t status			= range_cursor->next();

	while (status) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, min_key, range_cursor->getKey());
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, range_cursor->getValue(), min_key);
		status = range_cursor->next();
		min_key++;
		records_found++;
	}

	PLANCK_UNIT_ASSERT_FALSE(tc, range_cursor->hasNext());
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, records_expected, records_found);
	delete range_cursor;
}

/**
@brief	Aggregate test to test range cursor query on all implementations.
*/
void
test_cpp_wrapper_range_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_range(tc, dict, 5, 10);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_range(tc, dict, 1, 3);
	delete dict;

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_range(tc, dict, 4, 18);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50);
	test_cpp_wrapper_range(tc, dict, 30, 36);
	delete dict;

	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	test_cpp_wrapper_range(tc, dict, 5, 7);
	delete dict;
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
	int total_records = 0;

	for (int i = 0; i < random_positive_num; i++) {
		dict->insert(i, i * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, dict->last_status.count);
	}

	Cursor<int, int> *all_rec_cursor	= dict->allRecords();

	int curr_key						= all_rec_cursor->getKey();
	int curr_value						= all_rec_cursor->getValue();

	PLANCK_UNIT_ASSERT_TRUE(tc, all_rec_cursor->hasNext());
	all_rec_cursor->next();

	while (all_rec_cursor->hasNext()) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, total_records, all_rec_cursor->getKey());
		PLANCK_UNIT_ASSERT_TRUE(tc, all_rec_cursor->getValue() == total_records * 2);
		PLANCK_UNIT_ASSERT_TRUE(tc, all_rec_cursor->hasNext());
		all_rec_cursor->next();
		total_records++;
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, !all_rec_cursor->hasNext());

	/* Check that same number of records are found as were inserted with desired key. */
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, random_positive_num, total_records);
	delete all_rec_cursor;
}

/**
@brief	Aggregate test to test all records cursor query on all implementations.
*/
void
test_cpp_wrapper_all_records_on_all_implementations(
	planck_unit_test_t *tc
) {
	Dictionary<int, int> *dict;

	dict = new BppTree<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_all_records(tc, dict, 10);
	delete dict;

	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7);
	test_cpp_wrapper_all_records(tc, dict, 4);
	delete dict;

	/* The following dictionary implementations do not yet have working all record query abilities. */

	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int));
	test_cpp_wrapper_all_records(tc, dict, 13);
	delete dict;

	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 10);
	test_cpp_wrapper_all_records(tc, dict, 5);
	delete dict;

/*	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50); */
/*	test_cpp_wrapper_all_records(tc, dict, 8); */
/*	delete dict; */
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
	ion_status_t		status;
	err_t				error;
	ion_dictionary_id_t gdict_id	= dict->dict.instance->id;
	int					key_size	= dict->dict.instance->record.key_size;
	int					val_size	= dict->dict.instance->record.value_size;
	key_type_t			key_type	= dict->dict.instance->key_type;
	int					dict_size	= dict->dict_size;

	/* Insert test record so we can check data integrity after we close/open */
	status = dict->insert(key, value);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	error = dict->close();

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);

	ion_dictionary_config_info_t config = {
		gdict_id, 0, key_type, key_size, val_size, dict_size
	};

	error = dict->open(config);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict->dict.instance->record.key_size == key_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, dict->dict.instance->record.value_size == val_size);

	/* Check the test record */
	int ret_val = dict->get(key);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == dict->last_status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == dict->last_status.count);
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

/*	dict = new SkipList<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 7); */
/*	test_cpp_wrapper_open_close(tc, dict, 1, 13); */
/*	delete dict; */

/*	dict = new FlatFile<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int)); */
/*	test_cpp_wrapper_open_close(tc, dict, 45, 14); */
/*	delete dict; */

/*	dict = new OpenAddressHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50); */
/*	test_cpp_wrapper_open_close(tc, dict, 3, 15); */
/*	delete dict; */

/*	dict = new OpenAddressFileHash<int, int>(key_type_numeric_signed, sizeof(int), sizeof(int), 50); */
/*	test_cpp_wrapper_open_close(tc, dict, 11, 16); */
/*	delete dict; */
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
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_delete_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_update_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_equality_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_range_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_all_records_on_all_implementations);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_cpp_wrapper_open_close_on_all_implementations);

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
