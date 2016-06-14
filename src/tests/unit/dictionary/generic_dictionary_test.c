#include "generic_dictionary_test.h"

void
init_generic_dictionary_test(
	generic_test_t			*test,
	void (					*init_dict_handler)(dictionary_handler_t *),
	key_type_t				key_type,
	ion_key_size_t			key_size,
	ion_value_size_t		value_size,
	ion_dictionary_size_t	dictionary_size
) {
	test->init_dict_handler = init_dict_handler;
	test->key_type			= key_type;
	test->key_size			= key_size;
	test->value_size		= value_size;
	test->dictionary_size	= dictionary_size;
}

void
cleanup_generic_dictionary_test(
	generic_test_t *test
) {
	dictionary_delete_dictionary(&test->dictionary);
}

void
dictionary_test_init(
	generic_test_t		*test,
	planck_unit_test_t	*tc
) {
	err_t error;

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
	generic_test_t		*test,
	int					num_to_insert,
	planck_unit_test_t	*tc
) {
	if ((0 > num_to_insert) || (num_to_insert * test->key_size > INT_MAX - 1000) || (num_to_insert * test->value_size > INT_MAX - 1000)) {
		num_to_insert = 10;
	}

	byte	keys[num_to_insert * test->key_size];
	byte	vals[num_to_insert * test->value_size];

	unsigned char	test_buf[test->value_size];
	ion_value_t		test_val = test_buf;
	err_t			err;

	int i;
	int j;

	for (i = 0; i < num_to_insert; i++) {
		for (j = 0; j < test->key_size; j++) {
			keys[j] = 0x0;
		}

		for (j = 0; j < test->value_size; j++) {
			vals[j] = 0;
		}

		j = test->key_size;

		if (j > sizeof(int)) {
			j = sizeof(int);
		}

		memcpy(&keys[(i * test->key_size)], &i, j);

		j = sizeof(int);

		if (j > test->value_size) {
			j = test->value_size;
		}

		memcpy(&vals[(i * test->value_size)], &i, j);

		err = dictionary_insert(&(test->dictionary), &keys[(i * test->key_size)], &vals[(i * test->value_size)]);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == err);
	}

	for (i = 0; i < num_to_insert; i++) {
		err = dictionary_get(&(test->dictionary), &keys[(i * test->key_size)], test_val);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == err);

		j = memcmp(&(vals[i * test->value_size]), test_val, test->value_size);

		PLANCK_UNIT_ASSERT_TRUE(tc, 0 == j);
	}
}

void
dictionary_test_insert_get_edge_cases(
	generic_test_t		*test,
	planck_unit_test_t	*tc
) {
	err_t err;

	err = dictionary_insert(&(test->dictionary), IONIZE(-10, int), GTEST_DATA);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == err);

	unsigned char test_buf[test->value_size];

	err = dictionary_get(&test->dictionary, IONIZE(-10, int), test_buf);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == err);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == test->dictionary.instance->compare(GTEST_DATA, test_buf, test->key_size));
}

void
dictionary_test_delete(
	generic_test_t		*test,
	ion_key_t			key_to_delete,
	planck_unit_test_t	*tc
) {
	err_t err;

	unsigned char	test_buf[test->value_size];
	ion_value_t		test_val = test_buf;

	err = dictionary_delete(&(test->dictionary), key_to_delete);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == err);

	err = dictionary_get(&(test->dictionary), key_to_delete, test_val);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == err);
}

void
dictionary_test_update(
	generic_test_t		*test,
	ion_key_t			key_to_update,
	ion_value_t			update_with,
	planck_unit_test_t	*tc
) {
	err_t err;

	unsigned char	test_buf[test->value_size];
	ion_value_t		test_val = test_buf;

	err = dictionary_update(&(test->dictionary), key_to_update, update_with);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == err);

	err = dictionary_get(&(test->dictionary), key_to_update, test_val);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == err);

	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == memcmp(update_with, test_val, test->value_size));
}

void
dictionary_test_equality(
	generic_test_t		*test,
	ion_key_t			eq_key,
	planck_unit_test_t	*tc
) {
	err_t error;

	dict_cursor_t	*cursor = NULL;
	predicate_t		predicate;

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
	generic_test_t		*test,
	ion_key_t			lower_bound,
	ion_key_t			upper_bound,
	planck_unit_test_t	*tc
) {
	err_t error;

	dict_cursor_t	*cursor = NULL;
	predicate_t		predicate;

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
	generic_test_t		*test,
	int					expected_count,
	planck_unit_test_t	*tc
) {
	err_t error;

	dict_cursor_t	*cursor = NULL;
	predicate_t		predicate;

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
	generic_test_t		*test,
	planck_unit_test_t	*tc
) {
	err_t				error;
	ion_dictionary_id_t gdict_id = test->dictionary.instance->id;

	/* Insert test record so we can check data integrity after we close/open */
	error = dictionary_insert(&(test->dictionary), IONIZE(66650, int), GTEST_DATA);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);

	error = dictionary_close(&test->dictionary);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);

	dictionary_t			dictionary_temp;
	dictionary_handler_t	handler_temp;

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
	error = dictionary_get(&dictionary_temp, IONIZE(66650, int), test_val);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == error);

	int j = memcmp(GTEST_DATA, test_val, test->value_size);

	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == j);

	test->dictionary			= dictionary_temp;
	test->handler				= handler_temp;
	test->dictionary.handler	= &test->handler;
}
