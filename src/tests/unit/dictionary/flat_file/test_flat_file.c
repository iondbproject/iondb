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

ion_err_t
flat_file_scan(
	ion_flat_file_t				*flat_file,
	ion_fpos_t					*location,
	ion_boolean_t				scan_forwards,
	ion_flat_file_predicate_t	predicate
);

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
	ftest_create(tc, flat_file, key_type_numeric_signed, sizeof(int), sizeof(int), 1);
	flat_file->super.compare = dictionary_compare_signed_value;
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

planck_unit_suite_t *
flat_file_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, test_flat_file_create_destroy);

	return suite;
}

void
runalltests_flat_file(
) {
	planck_unit_suite_t *suite = flat_file_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
