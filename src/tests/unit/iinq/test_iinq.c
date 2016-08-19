#include "test_iinq.h"

typedef void (*iinq_test_results_func_t) (planck_unit_test_t *, ion_iinq_result_t*, uint32_t, uint32_t);

typedef struct {
	uint32_t 					total;
	uint32_t					count;
	planck_unit_test_t			*tc;
	iinq_test_results_func_t	func;
} iinq_test_query_state_t;

void
iinq_test_create_open_source(
	planck_unit_test_t	*tc,
	char				*schema_file_name,
	ion_key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size
) {
	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;
	ion_dictionary_id_t			id;

	error				= iinq_create_source(schema_file_name, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);
	error				= iinq_create_source(schema_file_name, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_duplicate_dictionary_error, error);

	dictionary.handler	= &handler;

	error				= iinq_open_source(schema_file_name, &dictionary, &handler);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);
	id					= dictionary.instance->id;
	ion_close_dictionary(&dictionary);

	char cleanup_name[20];

	sprintf(cleanup_name, "%d.bpt", (int) id);
	fremove(cleanup_name);
	sprintf(cleanup_name, "%d.val", (int) id);
	fremove(cleanup_name);
	sprintf(cleanup_name, "%d.bpt", (int) id); // TODO: is this correct?
	fremove(schema_file_name);
}

void
iinq_test_create_insert_update_delete_drop_dictionary(
	planck_unit_test_t	*tc,
	ion_key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size,
	ion_key_t			insert_key,
	ion_value_t			insert_value,
	ion_value_t			update_value
) {
	ion_err_t		error;
	ion_status_t	status;

	error	= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	error	= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_duplicate_dictionary_error, error);

	status	= INSERT(test, insert_key, insert_value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	status = UPDATE(test, insert_key, update_value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	status = DELETE(test, insert_key);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	error = DROP(test);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);
}

void
iinq_test_create_open_source_intint(
	planck_unit_test_t *tc
) {
	char				*schema_file_name;
	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	schema_file_name	= "intint.inq";
	key_type			= key_type_numeric_signed;
	key_size			= sizeof(int);
	value_size			= sizeof(int);

	iinq_test_create_open_source(tc, schema_file_name, key_type, key_size, value_size);
}

void
iinq_test_create_open_source_string10string20(
	planck_unit_test_t *tc
) {
	char				*schema_file_name;
	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	schema_file_name	= "s10s20.inq";
	key_type			= key_type_char_array;
	key_size			= 10;
	value_size			= 20;

	iinq_test_create_open_source(tc, schema_file_name, key_type, key_size, value_size);
}

void
iinq_test_create_insert_update_delete_drop_dictionary_intint(
	planck_unit_test_t *tc
) {
	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;
	ion_key_t			insert_key;
	ion_value_t			insert_value;
	ion_value_t			update_value;

	key_type		= key_type_numeric_signed;
	key_size		= sizeof(int);
	value_size		= sizeof(int);
	insert_key		= IONIZE(1, int);
	insert_value	= IONIZE(100, int);
	update_value	= IONIZE(101, int);

	iinq_test_create_insert_update_delete_drop_dictionary(tc, key_type, key_size, value_size, insert_key, insert_value, update_value);
}

IINQ_NEW_PROCESSOR_FUNC(print_hello) {
	UNUSED(result);
	UNUSED(state);
	printf("In here, hello!\n");
	fflush(stdout);
}

void
iinq_test_create_query_select_all_from_where_single_dictionary(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_status_t				status;
	ion_iinq_query_processor_t	processor;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;
	ion_key_t			key;
	ion_value_t			value;

	processor	= IINQ_QUERY_PROCESSOR(print_hello, NULL);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(int);
	value_size	= sizeof(int);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	key			= IONIZE(1, int);
	value		= IONIZE(1, int);

	status		= INSERT(test, key, value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	key		= IONIZE(1, int);
	value	= IONIZE(1, int);

	status	= INSERT(test, key, value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	QUERY(SELECT_ALL, FROM(0, test), WHERE(NEUTRALIZE(test.key, int) == 1), , , , , , &processor);

	DROP(test);
}

void
iinq_test_create_query_select_all_from_where_two_dictionaries(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_status_t				status;
	ion_iinq_query_processor_t	processor;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;
	ion_key_t			key;
	ion_value_t			value;

	processor	= IINQ_QUERY_PROCESSOR(print_hello, NULL);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(int);
	value_size	= sizeof(int);

	error		= CREATE_DICTIONARY(test1, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	error		= CREATE_DICTIONARY(test2, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	key			= IONIZE(1, int);
	value		= IONIZE(1, int);

	status		= INSERT(test1, key, value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	status = INSERT(test2, key, value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	key		= IONIZE(2, int);
	value	= IONIZE(2, int);

	status	= INSERT(test1, key, value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	status = INSERT(test2, key, value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	QUERY(SELECT_ALL, FROM(0, test1, test2), WHERE(NEUTRALIZE(test1.key, int) == 1), , , , , , &processor);

	DROP(test1);
	DROP(test2);
}

void
iinq_test_create_query_select_all_from_where_aggregates(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_status_t				status;
	ion_iinq_query_processor_t	processor;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;
	ion_key_t			key;
	ion_value_t			value;

	processor	= IINQ_QUERY_PROCESSOR(print_hello, NULL);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(int);
	value_size	= sizeof(int);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	key			= IONIZE(1, int);
	value		= IONIZE(1, int);

	status		= INSERT(test, key, value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	key		= IONIZE(2, int);
	value	= IONIZE(2, int);

	status	= INSERT(test, key, value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	MATERIALIZED_QUERY(SELECT_ALL, AGGREGATES(MAX(NEUTRALIZE(test.key, int))), FROM(0, test), WHERE(NEUTRALIZE(test.key, int) == 1), GROUPBY_NONE, /* HAVING */, ORDERBY_NONE, , , &processor);

	DROP(test);
}

IINQ_NEW_PROCESSOR_FUNC(check_results) {
	iinq_test_query_state_t	*query_state	= state;
	query_state->func(query_state->tc, result, query_state->count, query_state->total);
	query_state->count++;
}

void
iinq_test_create_query_select_all_from_where_orderby_descending_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t 			total
)
{
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint32_t) * 2, result->num_bytes);
	ion_key_t	key		= result->data;
	ion_value_t value	= result->data + sizeof(uint32_t);

	int expected_key = total - count - 1;
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_key, NEUTRALIZE(key, uint32_t));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_key, NEUTRALIZE(value, uint32_t));
}

void
iinq_test_create_query_select_all_from_where_orderby(
	planck_unit_test_t *tc,
	uint32_t num_records,
	ion_boolean_e ascending
) {
	ion_err_t					error;
	ion_status_t				status;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;
	state.count					= 0;
	state.tc					= tc;
	state.total					= num_records; /* This will write past a page boundary. */
	state.func					= iinq_test_create_query_select_all_from_where_orderby_descending_records;

	ion_key_type_t				key_type;
	ion_key_size_t				key_size;
	ion_value_size_t			value_size;
	ion_key_t					key;
	ion_value_t					value;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	uint32_t i;
	for (i = 0; i < state.total; i++) {
		key			= IONIZE(i, uint32_t);
		value		= IONIZE(1, uint32_t);

		status		= INSERT(test, key, value);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
		PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);
	}

	MATERIALIZED_QUERY(SELECT_ALL, AGGREGATES_NONE, FROM(0, test), WHERE(1), GROUPBY_NONE, , ORDERBY(DESCENDING(NEUTRALIZE(test.key, uint32_t))) , , , &processor);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, state.total, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_all_from_where_orderby_ascending_small(
	planck_unit_test_t *tc
) {

}

void
iinq_test_create_query_select_all_from_where_orderby_ascending_med(
	planck_unit_test_t *tc
) {

}

void
iinq_test_create_query_select_all_from_where_orderby_ascending_large(
	planck_unit_test_t *tc
) {

}

void
iinq_test_create_query_select_all_from_where_orderby_descending_small(
	planck_unit_test_t *tc
) {
	iinq_test_create_query_select_all_from_where_orderby(tc, 2, boolean_false);
}

void
iinq_test_create_query_select_all_from_where_orderby_descending_med(
	planck_unit_test_t *tc
) {
	iinq_test_create_query_select_all_from_where_orderby(tc, 43, boolean_false);
}

void
iinq_test_create_query_select_all_from_where_orderby_descending_large(
	planck_unit_test_t *tc
) {
	iinq_test_create_query_select_all_from_where_orderby(tc, 1000, boolean_false);
}

void
iinq_test_create_query_select_all_from_where_groupby_aggregate_simple_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count
)
{
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(int)*2, result->num_bytes);
	ion_key_t	key		= result->data;
	ion_value_t value	= result->data + sizeof(int);
	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, NEUTRALIZE(key, int));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, NEUTRALIZE(value, int));
			break;
		case 1:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(key, int));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(value, int));
			break;
	}
}

void
iinq_test_create_query_select_all_from_where_groupby_aggregate_simple(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_status_t				status;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;
	state.count					= 0;
	state.tc					= tc;
	state.func					= iinq_test_create_query_select_all_from_where_groupby_aggregate_simple_records;

	ion_key_type_t				key_type;
	ion_key_size_t				key_size;
	ion_value_size_t			value_size;
	ion_key_t					key;
	ion_value_t					value;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(int);
	value_size	= sizeof(int);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	key			= IONIZE(1, int);
	value		= IONIZE(1, int);

	status		= INSERT(test, key, value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	key			= IONIZE(2, int);
	value		= IONIZE(2, int);

	status		= INSERT(test, key, value);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);

	MATERIALIZED_QUERY(
		SELECT_ALL,
		AGGREGATES(MAX(NEUTRALIZE(test.key, int))),
		FROM(0, test),
		WHERE(1),
		GROUPBY(ASC(NEUTRALIZE(test.key, int)))
		,
		/* HAVING */,
		ORDERBY_NONE
		, , ,
		&processor
	);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, state.count);

	DROP(test);
}

planck_unit_suite_t *
iinq_get_suite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_open_source_intint);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_open_source_string10string20);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_insert_update_delete_drop_dictionary_intint);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_single_dictionary);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_two_dictionaries);
//	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_aggregates);
//	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_orderby_ascending_small);
//	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_orderby_ascending_med);
//	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_orderby_ascending_large);
//	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_orderby_descending_small);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_orderby_descending_med);
//	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_orderby_descending_large);

//	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_groupby_aggregate_simple);

	return suite;
}

void
run_all_tests_iinq(
) {
	planck_unit_suite_t *suite = iinq_get_suite();

	planck_unit_run_suite(suite);

	planck_unit_destroy_suite(suite);

	fremove(ION_MASTER_TABLE_FILENAME);
}
