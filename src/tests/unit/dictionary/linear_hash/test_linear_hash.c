/******************************************************************************/
/**
@file
@author		Spencer MacBeth
@brief		Implementation unit tests for the linear hash.
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

#include "test_linear_hash.h"

void
test_linear_hash_create_destroy(
	planck_unit_test_t *tc
) {
	PLANCK_UNIT_ASSERT_TRUE(tc, boolean_true);
}

/**
@brief		Initializes a test flatfile instance and does a few sanity checks.
*/
void
test_linear_hash_create(
	planck_unit_test_t		*tc,
	linear_hash_table_t		*linear_hash,
	ion_key_type_t			key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size
) {
	/* TODO create and desetroy a linear_hash in every file */

	int				initial_size		= 5;
	int				split_threshold		= 85;
	int				records_per_bucket	= 4;
	array_list_t	*bucket_map			= alloca(sizeof(array_list_t));

	ion_err_t err						= linear_hash_init(1, dictionary_size, key_type, key_size, value_size, initial_size, split_threshold, records_per_bucket, bucket_map, linear_hash);

	linear_hash->super.compare = dictionary_compare_signed_value;

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, err);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != linear_hash->database);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != linear_hash->state);
	PLANCK_UNIT_ASSERT_TRUE(tc, initial_size == linear_hash->num_buckets);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == linear_hash->num_records);

	err = linear_hash_close(linear_hash);	/* todo test me */
}

planck_unit_suite_t *
linear_hash_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	return suite;
}

void
runalltests_linear_hash(
) {
	planck_unit_suite_t *suite = linear_hash_getsuite();

	planck_unit_run_suite(suite);
	planck_unit_destroy_suite(suite);
}
