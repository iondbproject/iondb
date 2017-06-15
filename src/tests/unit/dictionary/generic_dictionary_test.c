/******************************************************************************/
/**
@file		generic_dictionary_test.c
@author		IonDB Project
@brief		Entry point for dictionnary tests.
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

#include "generic_dictionary_test.h"

int
get_count_index_by_key(
	ion_key_t			needle,
	ion_key_t			*haystack,
	int					length,
	ion_dictionary_t	*dictionary
) {
	int i = 0;

	for (; i < length; i++) {
		if (0 == dictionary->instance->compare(needle, haystack[i], dictionary->instance->record.key_size)) {
			return i;
		}
	}

	return -1;
}

void
init_generic_dictionary_test(
	ion_generic_test_t			*test,
	ion_handler_initializer_t	init_dict_handler,
	ion_key_type_t				key_type,
	ion_key_size_t				key_size,
	ion_value_size_t			value_size,
	ion_dictionary_size_t		dictionary_size
) {
	test->init_dict_handler = init_dict_handler;
	test->key_type			= key_type;
	test->key_size			= key_size;
	test->value_size		= value_size;
	test->dictionary_size	= dictionary_size;
}

void
cleanup_generic_dictionary_test(
	ion_generic_test_t *test
) {
	dictionary_delete_dictionary(&test->dictionary);
}

void
dictionary_test_init(
	ion_generic_test_t	*test,
	planck_unit_test_t	*tc
) {
	ion_err_t error;

	test->init_dict_handler(&(test->handler));

	error = dictionary_create(&test->handler, &test->dictionary, 1, test->key_type, test->key_size, test->value_size, test->dictionary_size);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != test->dictionary.handler);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != test->dictionary.instance);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL != test->dictionary.instance->compare);
	PLANCK_UNIT_ASSERT_TRUE(tc, test->dictionary.instance->key_type == test->key_type);
	PLANCK_UNIT_ASSERT_TRUE(tc, test->dictionary.instance->record.key_size == test->key_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, test->dictionary.instance->record.value_size == test->value_size);
}

void
dictionary_test_insert_get(
	ion_generic_test_t	*test,
	int					num_to_insert,
	ion_key_t			*count_keys,
	ion_result_count_t	*counts,
	int					length,
	planck_unit_test_t	*tc
) {
	if ((0 > num_to_insert) || (num_to_insert * test->key_size > INT_MAX - 1000) || (num_to_insert * test->value_size > INT_MAX - 1000)) {
		num_to_insert = 10;
	}

	ion_byte_t	keys[num_to_insert * test->key_size];
	ion_byte_t	vals[num_to_insert * test->value_size];

	ion_byte_t		test_buf[test->value_size];
	ion_value_t		test_val = test_buf;
	ion_status_t	status;

	int i;
	int j;
	int k;

	for (i = 0; i < num_to_insert; i++) {
		for (j = 0; j < test->key_size; j++) {
			keys[j] = 0x0;
		}

		for (j = 0; j < test->value_size; j++) {
			vals[j] = 0;
		}

		j = test->key_size;

		if ((unsigned) j > sizeof(int)) {
			j = sizeof(int);
		}

		memcpy(&keys[(i * test->key_size)], &i, j);

		j = sizeof(int);

		if (j > test->value_size) {
			j = test->value_size;
		}

		memcpy(&vals[(i * test->value_size)], &i, j);

		status	= dictionary_insert(&(test->dictionary), &keys[(i * test->key_size)], &vals[(i * test->value_size)]);
		k		= get_count_index_by_key(&keys[(i * test->key_size)], count_keys, length, &(test->dictionary));

		if ((-1 != k) && (err_ok == status.error) && (1 == status.count)) {
			counts[k]++;
		}

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	for (i = 0; i < num_to_insert; i++) {
		status = dictionary_get(&(test->dictionary), &keys[(i * test->key_size)], test_val);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		j = memcmp(&(vals[i * test->value_size]), test_val, test->value_size);

		PLANCK_UNIT_ASSERT_TRUE(tc, 0 == j);
	}
}

void
dictionary_test_insert_get_edge_cases(
	ion_generic_test_t	*test,
	ion_key_t			*count_keys,
	ion_result_count_t	*counts,
	int					length,
	planck_unit_test_t	*tc
) {
	ion_status_t	status;
	int				k;
	ion_key_t		key = IONIZE(-10, int);

	status	= dictionary_insert(&(test->dictionary), key, IONIZE(44, int));
	k		= get_count_index_by_key(key, count_keys, length, &(test->dictionary));

	if ((-1 != k) && (err_ok == status.error) && (1 == status.count)) {
		counts[k]++;
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	ion_byte_t test_buf[test->value_size];

	status = dictionary_get(&test->dictionary, IONIZE(-10, int), test_buf);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == test->dictionary.instance->compare(IONIZE(44, int), test_buf, test->value_size));
}

void
dictionary_test_delete(
	ion_generic_test_t	*test,
	ion_key_t			key_to_delete,
	ion_result_count_t	count,
	planck_unit_test_t	*tc
) {
	ion_status_t status;

	ion_byte_t	test_buf[test->value_size];
	ion_value_t test_val = test_buf;

	status = dictionary_delete(&(test->dictionary), key_to_delete);

	/* If we don't expect to delete anything, then we expect to see that the item didn't exist. */
	if (0 == count) {
		PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	}
	else {
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, count == status.count);

	status = dictionary_get(&(test->dictionary), key_to_delete, test_val);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);
}

void
dictionary_test_update(
	ion_generic_test_t	*test,
	ion_key_t			key_to_update,
	ion_value_t			update_with,
	ion_result_count_t	count,
	planck_unit_test_t	*tc
) {
	ion_status_t status;

	ion_byte_t	test_buf[test->value_size];
	ion_value_t test_val = test_buf;

	status = dictionary_update(&(test->dictionary), key_to_update, update_with);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, count == status.count);

	status = dictionary_get(&(test->dictionary), key_to_update, test_val);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, count == status.count);

	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == memcmp(update_with, test_val, test->value_size));
}

void
dictionary_test_equality(
	ion_generic_test_t	*test,
	ion_key_t			eq_key,
	planck_unit_test_t	*tc
) {
	ion_err_t error;

	ion_dict_cursor_t	*cursor = NULL;
	ion_predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_equality, eq_key);
	error = dictionary_find(&test->dictionary, &predicate, &cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;

	record.key		= malloc(test->key_size);
	record.value	= malloc(test->value_size);

	while (cs_end_of_results != cursor->next(cursor, &record)) {
		PLANCK_UNIT_ASSERT_TRUE(tc, test->dictionary.instance->compare(record.key, eq_key, test->key_size) == 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->status);

	free(record.key);
	free(record.value);

	cursor->destroy(&cursor);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);
}

void
dictionary_test_range(
	ion_generic_test_t	*test,
	ion_key_t			lower_bound,
	ion_key_t			upper_bound,
	planck_unit_test_t	*tc
) {
	ion_err_t error;

	ion_dict_cursor_t	*cursor = NULL;
	ion_predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_range, lower_bound, upper_bound);
	error = dictionary_find(&test->dictionary, &predicate, &cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;

	record.key		= malloc(test->key_size);
	record.value	= malloc(test->value_size);

	while (cs_end_of_results != cursor->next(cursor, &record)) {
		PLANCK_UNIT_ASSERT_TRUE(tc, test->dictionary.instance->compare(record.key, lower_bound, test->key_size) >= 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, test->dictionary.instance->compare(record.key, upper_bound, test->key_size) <= 0);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->status);

	free(record.key);
	free(record.value);

	cursor->destroy(&cursor);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);
}

void
dictionary_test_all_records(
	ion_generic_test_t	*test,
	int					expected_count,
	planck_unit_test_t	*tc
) {
	ion_err_t error;

	ion_dict_cursor_t	*cursor = NULL;
	ion_predicate_t		predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);
	error = dictionary_find(&test->dictionary, &predicate, &cursor);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);
	PLANCK_UNIT_ASSERT_TRUE(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;

	record.key		= malloc(test->key_size);
	record.value	= malloc(test->value_size);

	int count = 0;

	while (cs_end_of_results != cursor->next(cursor, &record)) {
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);
		count++;
	}

	if (expected_count >= 0) {
		PLANCK_UNIT_ASSERT_TRUE(tc, expected_count == count);
	}

	PLANCK_UNIT_ASSERT_TRUE(tc, cs_end_of_results == cursor->status);

	free(record.key);
	free(record.value);

	cursor->destroy(&cursor);
	PLANCK_UNIT_ASSERT_TRUE(tc, NULL == cursor);
}

void
dictionary_test_open_close(
	ion_generic_test_t	*test,
	planck_unit_test_t	*tc
) {
	ion_err_t			error;
	ion_status_t		status;
	ion_dictionary_id_t gdict_id = test->dictionary.instance->id;

	/* Insert test record so we can check data integrity after we close/open */
	status = dictionary_insert(&(test->dictionary), IONIZE(2379, int), ION_GTEST_DATA);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	error = dictionary_close(&test->dictionary);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);

	ion_dictionary_t			dictionary_temp;
	ion_dictionary_handler_t	handler_temp;

	test->init_dict_handler(&handler_temp);

	ion_dictionary_config_info_t config = {
		gdict_id, 0, test->key_type, test->key_size, test->value_size, test->dictionary_size
	};

	error = dictionary_open(&handler_temp, &dictionary_temp, &config);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);
	PLANCK_UNIT_ASSERT_TRUE(tc, dictionary_temp.instance->record.key_size == test->key_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, dictionary_temp.instance->record.value_size == test->value_size);

	/* Check the test record */
	IONIZE_VAL(test_val, test->value_size);
	status = dictionary_get(&dictionary_temp, IONIZE(2379, int), test_val);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	int j = memcmp(ION_GTEST_DATA, test_val, test->value_size);

	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == j);

	test->dictionary			= dictionary_temp;
	test->handler				= handler_temp;
	test->dictionary.handler	= &test->handler;
}
