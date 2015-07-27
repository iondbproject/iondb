#include "generic_dictionary_test.h"

void
init_generic_dictionary_test(
	generic_test_t		*test,
	void			(*init_dict_handler)(dictionary_handler_t *),
	key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size,
	ion_dictionary_size_t	dictionary_size
)
{
	test->init_dict_handler	= init_dict_handler;
	test->key_type		= key_type;
	test->key_size		= key_size;
	test->value_size	= value_size;
	test->dictionary_size	= dictionary_size;

	ion_init_master_table();
}

void
cleanup_generic_dictionary_test(
    generic_test_t      *test
)
{
	dictionary_delete_dictionary(&test->dictionary);
	ion_close_master_table();
}


void
dictionary_test_init(
	generic_test_t		*test,
	CuTest				*tc
)
{
	err_t			error;

	test->init_dict_handler(&(test->handler));

	error = ion_master_table_create_dictionary(
		&test->handler,
		&test->dictionary,
		test->key_type,
		test->key_size,
		test->value_size,
		test->dictionary_size
	);
	
	CuAssertTrue(tc, err_ok == error);
	CuAssertTrue(tc, NULL != test->dictionary.handler);
	CuAssertTrue(tc, NULL != test->dictionary.instance);
	CuAssertTrue(tc, NULL != test->dictionary.instance->compare);
	CuAssertTrue(tc, test->dictionary.instance->key_type == test->key_type);
	CuAssertTrue(tc, test->dictionary.instance->record.key_size == test->key_size);
	CuAssertTrue(tc, test->dictionary.instance->record.value_size == test->value_size);
}

void
dictionary_test_insert_get(
	generic_test_t	*test,
	int		num_to_insert,
	CuTest		*tc
)
{
	srand(time(NULL));
	
	if (0 > num_to_insert ||
	    num_to_insert * test->key_size > INT_MAX - 1000 ||
	    num_to_insert * test->value_size > INT_MAX - 1000
	)
	{
		num_to_insert = 10;
	}
	
	byte		keys[num_to_insert * test->key_size];
	byte		vals[num_to_insert * test->value_size];

	unsigned char 	test_buf[test->value_size];
	ion_value_t		test_val = test_buf;
	err_t			error;
	
	int		i;
	int		j;
	
	for (i = 0; i < num_to_insert; i++)
	{

		for (j = 0; j < test->key_size; j++)
		{
			keys[j]	= 0x0;
		}
		for (j = 0; j < test->value_size; j++)
		{
			vals[j]		= 0;
		}
		
		j		= test->key_size;
		if (j > sizeof(int))
			j	= sizeof(int);
		
		memcpy(
			&keys[(i*test->key_size)],
			&i,
			j
		);
		
		j		= sizeof(int);
		if (j > test->value_size)
			j 	= test->value_size;
		
		memcpy(
			&vals[(i*test->value_size)],
			&i,
			j
		);
		
		error	= dictionary_insert(
				&(test->dictionary),
				&keys[(i*test->key_size)],
				&vals[(i*test->value_size)]
			);
		CuAssertTrue(tc, err_ok == error);
	}
	
	for (i = 0; i < num_to_insert; i++)
	{
		error	= dictionary_get(
				&(test->dictionary),
				&keys[(i*test->key_size)],
				test_val
			);
		
		CuAssertTrue(tc, err_ok == error);
		
		j	= memcmp(
				&(vals[i*test->value_size]),
				test_val,
				test->value_size
			);

		CuAssertTrue(tc, 0 == j);
	}
}

void
dictionary_test_insert_get_edge_cases(
    generic_test_t  *test,
    CuTest      *tc
)
{
	err_t error;

	error = dictionary_insert(
		&(test->dictionary),
		IONIZE(-10),
		GTEST_DATA
	);

	CuAssertTrue(tc, err_ok == error);

	unsigned char 	test_buf[test->value_size];
	error = dictionary_get(
		&test->dictionary,
		IONIZE(-10),
		test_buf
		);

	CuAssertTrue(tc, err_ok == error);
	CuAssertTrue(tc, 0 == test->dictionary.instance->compare(GTEST_DATA, test_buf, test->key_size));
}

void
dictionary_test_delete(
	generic_test_t	*test,
	ion_key_t	key_to_delete,
	CuTest		*tc
)
{
	err_t			error;
	unsigned char 	test_buf[test->value_size];
	ion_value_t		test_val = test_buf;
	
	error	= dictionary_delete(
			&(test->dictionary),
			key_to_delete
		);
	
	CuAssertTrue(tc, err_ok == error);
	
	error	= dictionary_get(
			&(test->dictionary),
			key_to_delete,
			test_val
		);
	
	CuAssertTrue(tc, err_item_not_found == error);
}

void
dictionary_test_update(
	generic_test_t	*test,
	ion_key_t	key_to_update,
	ion_value_t	update_with,
	CuTest		*tc
)
{
	err_t			error;
	unsigned char 	test_buf[test->value_size];
	ion_value_t		test_val = test_buf;
	
	error	= dictionary_update(
			&(test->dictionary),
			key_to_update,
			update_with
		);
	
	CuAssertTrue(tc, err_ok == error);
	
	error	= dictionary_get(
			&(test->dictionary),
			key_to_update,
			test_val
		);
	
	CuAssertTrue(tc, err_ok == error);
	
	CuAssertTrue(tc, 0 == memcmp(update_with, test_val, test->value_size));

}

void
dictionary_test_equality(
    generic_test_t 	*test,
    ion_key_t 		eq_key,
    CuTest			*tc
)
{
	err_t error;

	dict_cursor_t 	*cursor 	= NULL;
	predicate_t 	*predicate 	= NULL;
	dictionary_build_predicate(&test->dictionary, &predicate, predicate_equality, eq_key);
	error = dictionary_find(&test->dictionary, predicate, &cursor);

	CuAssertTrue(tc, err_ok == error);
	CuAssertTrue(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;
	record.key 		= malloc(test->key_size);
	record.value 	= malloc(test->value_size);

	while(cs_end_of_results != cursor->next(cursor, &record))
	{
		CuAssertTrue(tc, test->dictionary.instance->compare(record.key, eq_key, test->key_size) == 0);
		CuAssertTrue(tc, err_ok == error);
	}

	CuAssertTrue(tc, cs_end_of_results == cursor->status);

	free(record.key);
	free(record.value);

	cursor->destroy(&cursor);
	CuAssertTrue(tc, NULL == cursor);
}

void
dictionary_test_range(
    generic_test_t 	*test,
    ion_key_t 		leq_key,
    ion_key_t 		geq_key,
    CuTest			*tc
)
{
	err_t error;

	dict_cursor_t 	*cursor 	= NULL;
	predicate_t 	*predicate 	= NULL;
	dictionary_build_predicate(&test->dictionary, &predicate, predicate_range, leq_key, geq_key);
	error = dictionary_find(&test->dictionary, predicate, &cursor);

	CuAssertTrue(tc, err_ok == error);
	CuAssertTrue(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;
	record.key 		= malloc(test->key_size);
	record.value 	= malloc(test->value_size);

	while(cs_end_of_results != cursor->next(cursor, &record))
	{
		CuAssertTrue(tc, test->dictionary.instance->compare(record.key, leq_key, test->key_size) >= 0);
		CuAssertTrue(tc, test->dictionary.instance->compare(record.key, geq_key, test->key_size) <= 0);
		CuAssertTrue(tc, err_ok == error);
	}

	CuAssertTrue(tc, cs_end_of_results == cursor->status);

	free(record.key);
	free(record.value);

	cursor->destroy(&cursor);
	CuAssertTrue(tc, NULL == cursor);
}

void
dictionary_test_all_records(
    generic_test_t 	*test,
    int				expected_count,
    CuTest			*tc
)
{
	err_t error;

	dict_cursor_t 	*cursor 	= NULL;
	predicate_t 	*predicate 	= NULL;
	dictionary_build_predicate(&test->dictionary, &predicate, predicate_all_records);
	error = dictionary_find(&test->dictionary, predicate, &cursor);

	CuAssertTrue(tc, err_ok == error);
	CuAssertTrue(tc, cs_cursor_initialized == cursor->status);

	ion_record_t record;
	record.key 		= malloc(test->key_size);
	record.value 	= malloc(test->value_size);
	int count	= 0;

	while(cs_end_of_results != cursor->next(cursor, &record))
	{
		CuAssertTrue(tc, err_ok == error);
		count++;
	}
	if (expected_count >= 0)
	{
		CuAssertTrue(tc, expected_count == count);
	}

	CuAssertTrue(tc, cs_end_of_results == cursor->status);

	free(record.key);
	free(record.value);

	cursor->destroy(&cursor);
	CuAssertTrue(tc, NULL == cursor);
}

void
dictionary_test_open_close(
    generic_test_t  *test,
    CuTest			*tc
)
{
	err_t error;
	ion_dictionary_id_t gdict_id = test->dictionary.instance->id;

	/* Insert test record so we can check data integrity after we close/open */
	error	= dictionary_insert(&(test->dictionary), IONIZE(66650), GTEST_DATA);

	CuAssertTrue(tc, err_ok == error);

	error = ion_close_dictionary(&test->dictionary);

	CuAssertTrue(tc, err_ok == error);

	dictionary_t 			dictionary_temp;
	dictionary_handler_t 	handler_temp;

	test->init_dict_handler(&handler_temp);

	error = ion_open_dictionary(&handler_temp, &dictionary_temp, gdict_id);

	CuAssertTrue(tc, err_ok == error);
	CuAssertTrue(tc, dictionary_temp.instance->record.key_size == test->key_size);
	CuAssertTrue(tc, dictionary_temp.instance->record.value_size == test->value_size);

	/* Check the test record */
	IONIZE_VAL(test_val, test->value_size);
	error	= dictionary_get(&dictionary_temp, IONIZE(66650), test_val);

	CuAssertTrue(tc, err_ok == error);
	int j	= memcmp(GTEST_DATA, test_val, test->value_size);
	CuAssertTrue(tc, 0 == j);

	test->dictionary 			= dictionary_temp;
	test->handler 				= handler_temp;
	test->dictionary.handler 	= &test->handler;
}
