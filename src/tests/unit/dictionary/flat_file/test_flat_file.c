/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Implementation unit tests for the flat file store.
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

#include "test_flat_file.h"

/********* PRIVATE METHOD DECLARATIONS **********/

/************************************************/

/**
@brief		Initializes a test flatfile instance and does a few sanity checks.
*/
void
ftest_create(
	planck_unit_test_t		*tc,
	ion_flat_file_t			*flat_file,
	ion_key_type_t			key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size
) {
	ion_err_t err = flat_file_initialize(flat_file, 0, key_type, key_size, value_size, dictionary_size);

	flat_file->super.compare	= dictionary_compare_signed_value;
	flat_file->super.id			= 0;

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != flat_file->data_file);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, boolean_false, flat_file->sorted_mode);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, dictionary_size, flat_file->num_buffered);
}

/**
@brief		Destroys a flatfile instance and checks to ensure everything is cleaned up.
*/
void
ftest_destroy(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file
) {
	ion_err_t err = flat_file_destroy(flat_file);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == flat_file->data_file);
}

/**
@brief		Sets up the default test instance we're going to use.
*/
void
ftest_setup(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file
) {
	ftest_create(tc, flat_file, key_type_numeric_signed, sizeof(int), sizeof(int), 15);
}

/**
@brief		Sets up the default test instance we're going to use, using a sorted backend.
*/
void
ftest_setup_sorted(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file
) {
	ftest_create(tc, flat_file, key_type_numeric_signed, sizeof(int), sizeof(int), 15);
	flat_file->sorted_mode = boolean_true;
}

/**
@brief		Tears down the default test instance we're going to use.
*/
void
ftest_takedown(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file
) {
	ftest_destroy(tc, flat_file);
}

/**
@brief		Inserts into the flat file and optionally checks if the insert was OK
			by reading the data file. Don't turn on check_result unless you expect
			the insert to fully pass.
*/
void
ftest_insert(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file,
	ion_key_t			key,
	ion_value_t			value,
	ion_err_t			expected_status,
	ion_result_count_t	expected_count,
	ion_boolean_t		check_result
) {
	ion_status_t status = flat_file_insert(flat_file, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (check_result) {
		/* TODO: This makes a big assumption on the layout of the data file. Is there a way to eliminate this? */
		ion_byte_t expected_result[flat_file->row_size];

		memset(expected_result, FLAT_FILE_STATUS_OCCUPIED, sizeof(ion_flat_file_row_status_t));
		memcpy(expected_result + sizeof(ion_flat_file_row_status_t), key, flat_file->super.record.key_size);
		memcpy(expected_result + sizeof(ion_flat_file_row_status_t) + flat_file->super.record.key_size, value, flat_file->super.record.value_size);

		ion_byte_t read_buffer[flat_file->row_size];

		fseek(flat_file->data_file, flat_file->start_of_data, SEEK_SET);

		ion_fpos_t cur_index = 0;

		while (boolean_true) {
			if (1 != fread(read_buffer, flat_file->row_size, 1, flat_file->data_file)) {
				break;
			}

			if (0 == memcmp(read_buffer, expected_result, flat_file->row_size)) {
				ion_flat_file_row_t test_row;
				ion_err_t			err = flat_file_read_row(flat_file, cur_index, &test_row);

				PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
				PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, FLAT_FILE_STATUS_OCCUPIED, test_row.row_status);
				PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, memcmp(test_row.key, key, flat_file->super.record.key_size));
				PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, memcmp(test_row.value, value, flat_file->super.record.value_size));
				return;
			}

			cur_index++;
		}

		/* If we reach here that means that the record we were looking for wasn't found :( */
		PLANCK_UNIT_SET_FAIL(tc);
	}
}

/**
@brief		Deletes from the flat file and asserts that the deletion was as expected.
*/
void
ftest_delete(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file,
	ion_key_t			key,
	ion_err_t			expected_status,
	ion_result_count_t	expected_count,
	ion_boolean_t		check_result
) {
	ion_status_t status = flat_file_delete(flat_file, key);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (check_result) {
		ion_fpos_t			loc = -1;
		ion_flat_file_row_t row;
		ion_err_t			err = flat_file_scan(flat_file, -1, &loc, &row, FLAT_FILE_SCAN_FORWARDS, flat_file_predicate_key_match, key);

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_file_hit_eof, err);
	}
}

/**
@brief		Gets from the flatfile and asserts everything went as expected.
*/
void
ftest_get(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file,
	ion_key_t			key,
	ion_err_t			expected_status,
	ion_value_t			expected_value
) {
	ion_value_t		value	= alloca(flat_file->super.record.value_size);
	ion_status_t	status	= flat_file_get(flat_file, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);

	if (err_ok == expected_status) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, memcmp(expected_value, value, flat_file->super.record.value_size));

		ion_fpos_t			loc = -1;
		ion_flat_file_row_t row;
		ion_err_t			err = flat_file_scan(flat_file, -1, &loc, &row, FLAT_FILE_SCAN_FORWARDS, flat_file_predicate_key_match, key);

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	}
}

/**
@brief		Updates in the flatfile and asserts everything went as expected.
*/
void
ftest_update(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file,
	ion_key_t			key,
	ion_value_t			value,
	ion_err_t			expected_status,
	ion_result_count_t	expected_count
) {
	ion_status_t status = flat_file_update(flat_file, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_count, status.count);

	if (err_ok == expected_status) {
		ion_fpos_t			loc = -1;
		ion_flat_file_row_t row;
		ion_err_t			err;

		while (err_ok == (err = flat_file_scan(flat_file, loc, &loc, &row, FLAT_FILE_SCAN_FORWARDS, flat_file_predicate_key_match, key))) {
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, memcmp(row.value, value, flat_file->super.record.value_size));
			loc++;
		}

		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_file_hit_eof, err);
	}
}

/**
@brief		Tests and asserts the correctness of the file scan method.
*/
void
ftest_file_scan(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file,
	ion_byte_t			scan_direction,
	ion_fpos_t			start_location,
	ion_key_t			target_key,
	ion_err_t			expected_status,
	ion_fpos_t			expected_location
) {
	ion_fpos_t			found_loc	= -1;
	ion_flat_file_row_t row;
	ion_err_t			err			= flat_file_scan(flat_file, start_location, &found_loc, &row, scan_direction, flat_file_predicate_key_match, target_key);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_location, found_loc);

	if (err_ok == expected_status) {
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 0, flat_file->super.compare(target_key, row.key, flat_file->super.record.key_size));
	}
}

/**
@brief		Tests several cases of a file scan.
*/
void
ftest_file_scan_cases(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file
) {
	ftest_insert(tc, flat_file, IONIZE(1, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(1, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(-50, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(2, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(3, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(-100, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(4, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(-150, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(5, int), IONIZE(0, int), err_ok, 1, boolean_true);

	/* Test start-on-return location cases */
	ftest_file_scan(tc, flat_file, boolean_true, 2, IONIZE(-50, int), err_ok, 2);
	ftest_file_scan(tc, flat_file, boolean_true, 8, IONIZE(5, int), err_ok, 8);
	ftest_file_scan(tc, flat_file, boolean_false, 4, IONIZE(3, int), err_ok, 4);
	ftest_file_scan(tc, flat_file, boolean_false, 8, IONIZE(5, int), err_ok, 8);

	/* Forwards from start | Forwards from middle | Forwards near end */
	ftest_file_scan(tc, flat_file, boolean_true, -1, IONIZE(-150, int), err_ok, 7);
	ftest_file_scan(tc, flat_file, boolean_true, 3, IONIZE(4, int), err_ok, 6);
	ftest_file_scan(tc, flat_file, boolean_true, 6, IONIZE(5, int), err_ok, 8);

	/* Backwards from end | Backwards from middle | Backwards near start */
	ftest_file_scan(tc, flat_file, boolean_false, -1, IONIZE(-50, int), err_ok, 2);
	ftest_file_scan(tc, flat_file, boolean_false, 4, IONIZE(2, int), err_ok, 3);
	ftest_file_scan(tc, flat_file, boolean_false, 1, IONIZE(1, int), err_ok, 1);

	/* Fallthrough forwards from start | Fallthrough forwards from middle | Fallthrough forwards near end */
	ftest_file_scan(tc, flat_file, boolean_true, -1, IONIZE(333, int), err_file_hit_eof, 9);
	ftest_file_scan(tc, flat_file, boolean_true, 3, IONIZE(333, int), err_file_hit_eof, 9);
	ftest_file_scan(tc, flat_file, boolean_true, 6, IONIZE(333, int), err_file_hit_eof, 9);

	/* Fallthrough backwards from end | Fallthrough backwards from middle | Fallthrough backwards near start */
	ftest_file_scan(tc, flat_file, boolean_false, -1, IONIZE(333, int), err_file_hit_eof, 9);
	ftest_file_scan(tc, flat_file, boolean_false, 4, IONIZE(333, int), err_file_hit_eof, 9);
	ftest_file_scan(tc, flat_file, boolean_false, 1, IONIZE(333, int), err_file_hit_eof, 9);

	/* Crazy start location cases */
	ftest_file_scan(tc, flat_file, boolean_true, -100, IONIZE(333, int), err_out_of_bounds, -1);
	ftest_file_scan(tc, flat_file, boolean_true, 100, IONIZE(333, int), err_out_of_bounds, -1);
	ftest_file_scan(tc, flat_file, boolean_false, -100, IONIZE(333, int), err_out_of_bounds, -1);
	ftest_file_scan(tc, flat_file, boolean_false, 100, IONIZE(333, int), err_out_of_bounds, -1);
}

/**
@brief		Tests and asserts the correctness of the binary search method.
*/
void
ftest_file_binary_search(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file,
	ion_key_t			target_key,
	ion_err_t			expected_status,
	ion_fpos_t			expected_location
) {
	ion_fpos_t	found_loc	= -1;
	ion_err_t	err			= flat_file_binary_search(flat_file, target_key, &found_loc);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_status, err);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_location, found_loc);
}

/**
@brief		Tests several cases of a binary search.
*/
void
ftest_file_binary_search_cases(
	planck_unit_test_t	*tc,
	ion_flat_file_t		*flat_file
) {
	ftest_insert(tc, flat_file, IONIZE(2, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(7, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(9, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(9, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(9, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(13, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(24, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(36, int), IONIZE(0, int), err_ok, 1, boolean_true);
	ftest_insert(tc, flat_file, IONIZE(99, int), IONIZE(0, int), err_ok, 1, boolean_true);

	/* Most typical search case */
	ftest_file_binary_search(tc, flat_file, IONIZE(13, int), err_ok, 5);
	/* Find a duplicate - should return the first in block */
	ftest_file_binary_search(tc, flat_file, IONIZE(9, int), err_ok, 2);
	/* Find missing from middle - should return the first less than */
	ftest_file_binary_search(tc, flat_file, IONIZE(12, int), err_ok, 4);
	/* Same as above but on lower end */
	ftest_file_binary_search(tc, flat_file, IONIZE(5, int), err_ok, 0);
	/* Same as above but on upper end */
	ftest_file_binary_search(tc, flat_file, IONIZE(130, int), err_ok, 8);
	/* Fall off bottom */
	ftest_file_binary_search(tc, flat_file, IONIZE(-5, int), err_item_not_found, -1);
}

/**
@brief		Tests some basic creation and destruction stuff for the flat file.
*/
void
test_flat_file_create_destroy(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup(tc, &flat_file);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a single insert statement.
*/
void
test_flat_file_insert_single(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(5, int), IONIZE(5, int), err_ok, 1, boolean_true);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests insertion of many things.
*/
void
test_flat_file_insert_many(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(5, int), IONIZE(1, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(10, int), IONIZE(2, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(4, int), IONIZE(3, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(500, int), IONIZE(4, int), err_ok, 1, boolean_true);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests all the different cases that the scan method can run into.
			With a small buffer size.
*/
void
test_flat_file_scan_cases_small_buf(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_create(tc, &flat_file, key_type_numeric_signed, sizeof(int), sizeof(int), 1);

	ftest_file_scan_cases(tc, &flat_file);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests all the different cases that the scan method can run into.
			With a large buffer size.
*/
void
test_flat_file_scan_cases_large_buf(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_create(tc, &flat_file, key_type_numeric_signed, sizeof(int), sizeof(int), 10);

	ftest_file_scan_cases(tc, &flat_file);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests the deletion edge case of deleting the last thing in the flat file.
*/
void
test_flat_file_delete_edge_case(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(5, int), IONIZE(1, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(10, int), IONIZE(2, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(4, int), IONIZE(3, int), err_ok, 1, boolean_true);

	ftest_delete(tc, &flat_file, IONIZE(4, int), err_ok, 1, boolean_true);

	ftest_insert(tc, &flat_file, IONIZE(23, int), IONIZE(3, int), err_ok, 1, boolean_true);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests an invalid insertion that would violate sorted order.
*/
void
test_flat_file_insert_bad_sort(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(5, int), IONIZE(1, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(3, int), IONIZE(99, int), err_sorted_order_violation, 0, boolean_false);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a valid insertion that does preserve sorted order.
*/
void
test_flat_file_insert_good_sort(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(5, int), IONIZE(1, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(76, int), IONIZE(99, int), err_ok, 1, boolean_true);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests several of the binary search cases to ensure they are all as expected.
*/
void
test_flat_file_sort_binary_search_cases(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_file_binary_search_cases(tc, &flat_file);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted get on an empty store.
*/
void
test_flat_file_sort_get_empty(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_get(tc, &flat_file, IONIZE(16, int), err_item_not_found, NULL);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted get on a 1 element store - it doesn't exist.
*/
void
test_flat_file_sort_get_single_nonexist(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(17, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_get(tc, &flat_file, IONIZE(33, int), err_item_not_found, NULL);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted get on a 1 element store - it does exist.
*/
void
test_flat_file_sort_get_single_exist(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(17, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_get(tc, &flat_file, IONIZE(17, int), err_ok, IONIZE(17, int));

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted get on a multi element store - it doesn't exist.
*/
void
test_flat_file_sort_get_many_nonexist(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(17, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(45, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(63, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(64, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(99, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(103, int), IONIZE(17, int), err_ok, 1, boolean_true);

	ftest_get(tc, &flat_file, IONIZE(33, int), err_item_not_found, NULL);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted get on a multi element store - it does exist.
*/
void
test_flat_file_sort_get_many_exist(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(17, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(45, int), IONIZE(1, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(63, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(64, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(99, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(103, int), IONIZE(17, int), err_ok, 1, boolean_true);

	ftest_get(tc, &flat_file, IONIZE(45, int), err_ok, IONIZE(1, int));

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted get on a multi element store with duplicates. It does exist.
*/
void
test_flat_file_sort_get_many_exist_duplicates(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(17, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(45, int), IONIZE(1, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(63, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(63, int), IONIZE(18, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(63, int), IONIZE(19, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(64, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(99, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(103, int), IONIZE(17, int), err_ok, 1, boolean_true);

	ftest_get(tc, &flat_file, IONIZE(63, int), err_ok, IONIZE(17, int));

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted update on an empty store.
*/
void
test_flat_file_sort_update_empty(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_update(tc, &flat_file, IONIZE(33, int), IONIZE(6, int), err_ok, 1);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted update on a 1 element store - it doesn't exist.
*/
void
test_flat_file_sort_update_single_nonexist(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(17, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_update(tc, &flat_file, IONIZE(33, int), IONIZE(1, int), err_ok, 1);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted update on a 1 element store - it does exist.
*/
void
test_flat_file_sort_update_single_exist(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(17, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_update(tc, &flat_file, IONIZE(17, int), IONIZE(64, int), err_ok, 1);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted update on a multi element store - it doesn't exist.
*/
void
test_flat_file_sort_update_many_nonexist(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(17, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(45, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(63, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(64, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(99, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(103, int), IONIZE(17, int), err_ok, 1, boolean_true);

	ftest_update(tc, &flat_file, IONIZE(43, int), IONIZE(91, int), err_sorted_order_violation, 0);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted update on a multi element store - it does exist.
*/
void
test_flat_file_sort_update_many_exist(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(17, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(45, int), IONIZE(1, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(63, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(64, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(99, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(103, int), IONIZE(17, int), err_ok, 1, boolean_true);

	ftest_update(tc, &flat_file, IONIZE(64, int), IONIZE(91, int), err_ok, 1);

	ftest_takedown(tc, &flat_file);
}

/**
@brief		Tests a sorted update on a multi element store with duplicates. It does exist.
*/
void
test_flat_file_sort_update_many_exist_duplicates(
	planck_unit_test_t *tc
) {
	ion_flat_file_t flat_file;

	ftest_setup_sorted(tc, &flat_file);

	ftest_insert(tc, &flat_file, IONIZE(17, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(45, int), IONIZE(1, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(63, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(63, int), IONIZE(18, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(63, int), IONIZE(19, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(64, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(99, int), IONIZE(17, int), err_ok, 1, boolean_true);
	ftest_insert(tc, &flat_file, IONIZE(103, int), IONIZE(17, int), err_ok, 1, boolean_true);

	ftest_update(tc, &flat_file, IONIZE(63, int), IONIZE(0, int), err_ok, 3);

	ftest_takedown(tc, &flat_file);
}

planck_unit_suite_t *
flat_file_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_create_destroy);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_insert_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_insert_many);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_scan_cases_small_buf);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_scan_cases_large_buf);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_delete_edge_case);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_insert_bad_sort);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_insert_good_sort);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_binary_search_cases);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_get_empty);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_get_single_nonexist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_get_single_exist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_get_many_nonexist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_get_many_exist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_get_many_exist_duplicates);

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_update_empty);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_update_single_nonexist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_update_single_exist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_update_many_nonexist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_update_many_exist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_sort_update_many_exist_duplicates);

	return suite;
}

void
runalltests_flat_file(
) {
	planck_unit_suite_t *suite = flat_file_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
