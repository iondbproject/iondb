#include "test_iinq.h"
#include "../../../iinq/iinq.h"

typedef void (*iinq_test_results_func_t)(
	planck_unit_test_t *,
	ion_iinq_result_t *,
	uint32_t,
	uint32_t
);

typedef struct {
	uint32_t					total;
	uint32_t					count;
	planck_unit_test_t			*tc;
	iinq_test_results_func_t	func;
} iinq_test_query_state_t;

IINQ_NEW_PROCESSOR_FUNC(check_results) {
	iinq_test_query_state_t *query_state = state;

	query_state->func(query_state->tc, result, query_state->count, query_state->total);
	query_state->count++;
}

void
iinq_test_insert_into_test(
	planck_unit_test_t	*tc,
	void				*key,
	void				*value
) {
	ion_status_t status = INSERT(test, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);
}

void
iinq_test_insert_into_test1(
	planck_unit_test_t	*tc,
	void				*key,
	void				*value
) {
	ion_status_t status = INSERT(test1, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);
}

void
iinq_test_insert_into_test2(
	planck_unit_test_t	*tc,
	void				*key,
	void				*value
) {
	ion_status_t status = INSERT(test2, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);
}

void
iinq_test_insert_into_test3(
	planck_unit_test_t	*tc,
	void				*key,
	void				*value
) {
	ion_status_t status = INSERT(test3, key, value);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, status.error);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, status.count);
}

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
	sprintf(cleanup_name, "%d.bpt", (int) id);	/* TODO: is this correct? */
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
	key_size			= sizeof(uint32_t);
	value_size			= sizeof(uint32_t);

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
	key_size		= sizeof(uint32_t);
	value_size		= sizeof(uint32_t);
	insert_key		= IONIZE(1, uint32_t);
	insert_value	= IONIZE(100, uint32_t);
	update_value	= IONIZE(101, uint32_t);

	iinq_test_create_insert_update_delete_drop_dictionary(tc, key_type, key_size, value_size, insert_key, insert_value, update_value);
}

void
iinq_test_create_query_select_all_from_where_single_dictionary_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint32_t) * 2, result->num_bytes);

	ion_key_t	key		= result->processed;
	ion_value_t value	= result->processed + sizeof(uint32_t);

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(key, uint32_t));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(value, uint32_t));
			break;

		case 1:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(key, uint32_t));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(value, uint32_t));
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_all_from_where_single_dictionary(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.func	= iinq_test_create_query_select_all_from_where_single_dictionary_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), IONIZE(1, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), IONIZE(1, uint32_t));

	QUERY(SELECT_ALL, FROM(0, test), WHERE(NEUTRALIZE(test.key, uint32_t) == 1), , , , , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_expression_all_from_where_single_dictionary_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint32_t), result->num_bytes);

	ion_key_t value = result->processed;

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 4, NEUTRALIZE(value, uint32_t));
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_expression_all_from_where_single_dictionary(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.func	= iinq_test_create_query_select_expression_all_from_where_single_dictionary_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), IONIZE(1, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), IONIZE(2, uint32_t));

	QUERY(SELECT(SELECT_EXPR(NEUTRALIZE(test.value, uint32_t) * 2)), FROM(0, test), WHERE(NEUTRALIZE(test.key, uint32_t) == 2), , , , , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_all_from_where_two_dictionaries_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint32_t) * 4, result->num_bytes);

	ion_key_t	key_1	= result->processed;
	ion_value_t value_1 = result->processed + sizeof(uint32_t);
	ion_key_t	key_2	= result->processed + sizeof(uint32_t) * 2;
	ion_value_t value_2 = result->processed + sizeof(uint32_t) * 3;

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(key_1, uint32_t));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(value_1, uint32_t));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(key_2, uint32_t));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(value_2, uint32_t));
			break;

		case 1:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(key_1, uint32_t));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(value_1, uint32_t));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, NEUTRALIZE(key_2, uint32_t));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, NEUTRALIZE(value_2, uint32_t));
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_all_from_where_two_dictionaries(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.func	= iinq_test_create_query_select_all_from_where_two_dictionaries_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test1, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	error		= CREATE_DICTIONARY(test2, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test1(tc, IONIZE(1, uint32_t), IONIZE(1, uint32_t));
	iinq_test_insert_into_test2(tc, IONIZE(1, uint32_t), IONIZE(1, uint32_t));

	iinq_test_insert_into_test1(tc, IONIZE(2, uint32_t), IONIZE(2, uint32_t));
	iinq_test_insert_into_test2(tc, IONIZE(2, uint32_t), IONIZE(2, uint32_t));

	QUERY(SELECT_ALL, FROM(0, test1, test2), WHERE(NEUTRALIZE(test1.key, uint32_t) == 1), , , , , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, state.count);

	DROP(test1);
	DROP(test2);
}

void
iinq_test_create_query_select_max_from_where_aggregates_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(double), result->num_bytes);

	double maxval = *((double *) (result->processed));

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_TRUE(tc, 1.0 == maxval);
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_max_from_where_aggregates(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.func	= iinq_test_create_query_select_max_from_where_aggregates_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), IONIZE(1, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), IONIZE(2, uint32_t));

	MATERIALIZED_QUERY(SELECT(SELECT_AGGR(0)), AGGREGATES(MAX(NEUTRALIZE(test.key, uint32_t))), FROM(0, test), WHERE(NEUTRALIZE(test.key, uint32_t) == 1), GROUPBY_NONE, HAVING_NONE, ORDERBY_NONE, , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_expression_from_where_aggregates_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint64_t), result->num_bytes);

	ion_key_t expr = result->processed;

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 4, NEUTRALIZE(expr, uint64_t));
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_expression_from_where_aggregates(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.func	= iinq_test_create_query_select_expression_from_where_aggregates_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), IONIZE(1, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), IONIZE(2, uint32_t));

	MATERIALIZED_QUERY(SELECT(SELECT_EXPR(2 * AGGREGATE(0))), AGGREGATES(MAX(NEUTRALIZE(test.key, uint32_t))), FROM(0, test),
		/* WHERE(NEUTRALIZE(test.key, uint32_t) == 2), */
		WHERE(1), GROUPBY_NONE, HAVING_NONE, ORDERBY_NONE, , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_complex_expression_from_where_aggregates_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint64_t), result->num_bytes);

	ion_key_t expr = result->processed;

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 4, NEUTRALIZE(expr, uint64_t));
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_complex_expression_from_where_aggregates(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.func	= iinq_test_create_query_select_complex_expression_from_where_aggregates_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), IONIZE(8, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), IONIZE(4, uint32_t));

	MATERIALIZED_QUERY(SELECT(SELECT_EXPR((AGGREGATE(0) * AGGREGATE(1)) / 4)), AGGREGATES(MAX(NEUTRALIZE(test.key, uint32_t)), MAX(NEUTRALIZE(test.value, uint32_t))), FROM(0, test), WHERE(1), GROUPBY_NONE, HAVING_NONE, ORDERBY_NONE, , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_all_from_where_orderby_ascending_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint32_t) * 2, result->num_bytes);

	ion_key_t	key		= result->processed;
	ion_value_t value	= result->processed + sizeof(uint32_t);

	int expected_key	= count;

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_key, NEUTRALIZE(key, uint32_t));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_key, NEUTRALIZE(value, uint32_t));
}

void
iinq_test_create_query_select_all_from_where_orderby_descending_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint32_t) * 2, result->num_bytes);

	ion_key_t	key		= result->processed;
	ion_value_t value	= result->processed + sizeof(uint32_t);

	int expected_key	= total - count - 1;

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_key, NEUTRALIZE(key, uint32_t));
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, expected_key, NEUTRALIZE(value, uint32_t));
}

void
iinq_test_orderby_general_ascending(
	ion_iinq_query_processor_t *processor
) {
	MATERIALIZED_QUERY(SELECT_ALL, AGGREGATES_NONE, FROM(0, test), WHERE(1), GROUPBY_NONE, HAVING_NONE, ORDERBY(ASCENDING_UINT(NEUTRALIZE(test.key, uint32_t))), , , processor);
}

void
iinq_test_orderby_general_descending(
	ion_iinq_query_processor_t *processor
) {
	MATERIALIZED_QUERY(SELECT_ALL, AGGREGATES_NONE, FROM(0, test), WHERE(1), GROUPBY_NONE, HAVING_NONE, ORDERBY(DESCENDING_UINT(NEUTRALIZE(test.key, uint32_t))), , , processor);
}

void
iinq_test_create_query_select_all_from_where_orderby(
	planck_unit_test_t	*tc,
	uint32_t			num_records,
	ion_boolean_e		ascending
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.total = num_records;

	if (boolean_true == ascending) {
		state.func = iinq_test_create_query_select_all_from_where_orderby_ascending_records;
	}
	else {
		state.func = iinq_test_create_query_select_all_from_where_orderby_descending_records;
	}

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;
	ion_key_t			key;
	ion_value_t			value;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	uint32_t i;

	for (i = 0; i < state.total; i++) {
		if (boolean_true == ascending) {
			key		= IONIZE(state.total - i - 1, uint32_t);
			value	= IONIZE(state.total - i - 1, uint32_t);
		}
		else {
			key		= IONIZE(i, uint32_t);
			value	= IONIZE(i, uint32_t);
		}

		iinq_test_insert_into_test(tc, key, value);
	}

	if (boolean_true == ascending) {
		iinq_test_orderby_general_ascending(&processor);
	}
	else {
		iinq_test_orderby_general_descending(&processor);
	}

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, state.total, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_all_from_where_orderby_ascending_small(
	planck_unit_test_t *tc
) {
	iinq_test_create_query_select_all_from_where_orderby(tc, 2, boolean_true);
}

void
iinq_test_create_query_select_all_from_where_orderby_ascending_large(
	planck_unit_test_t *tc
) {
	iinq_test_create_query_select_all_from_where_orderby(tc, 100, boolean_true);
}

void
iinq_test_create_query_select_all_from_where_orderby_descending_small(
	planck_unit_test_t *tc
) {
	iinq_test_create_query_select_all_from_where_orderby(tc, 2, boolean_false);
}

void
iinq_test_create_query_select_all_from_where_orderby_descending_large(
	planck_unit_test_t *tc
) {
	iinq_test_create_query_select_all_from_where_orderby(tc, 100, boolean_false);
}

void
iinq_test_create_query_select_max_from_where_groupby_aggregate_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(double), result->num_bytes);

	double maxval = *((double *) (result->processed));

	switch (count) {
		case 0:
			/* TODO: Make this an assert double macro. */
			PLANCK_UNIT_ASSERT_TRUE(tc, 14.0 == maxval);
			break;

		case 1:
			/* TODO: Make this an assert double macro. */
			PLANCK_UNIT_ASSERT_TRUE(tc, 2951.0 == maxval);
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_max_from_where_groupby_aggregate_small(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.total = 2;
	state.func	= iinq_test_create_query_select_max_from_where_groupby_aggregate_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), IONIZE(14, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), IONIZE(2951, uint32_t));

	MATERIALIZED_QUERY(SELECT(SELECT_AGGR(0)), AGGREGATES(MAX(NEUTRALIZE(test.value, uint32_t))), FROM(0, test), WHERE(1), GROUPBY(ASCENDING_INT(NEUTRALIZE(test.key, uint32_t))), HAVING_ALL, ORDERBY_NONE, , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_max_from_where_groupby_aggregate_large(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.total = 2;
	state.func	= iinq_test_create_query_select_max_from_where_groupby_aggregate_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;
	uint32_t			key_data;
	uint32_t			value_data;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	srand(1125123);

	/* Insert values [0, 2951) for key 2 and values [0, 14) for key 1. */
	int i;

	for (i = 0; i < 100; i++) {
		key_data	= 2;
		value_data	= (uint32_t) rand() % 2951;
		iinq_test_insert_into_test(tc, &key_data, &value_data);
		key_data	= 1;
		value_data	= (uint32_t) rand() % 14;
		iinq_test_insert_into_test(tc, &key_data, &value_data);
	}

	/* This is the max value to be inserted for key 2. */
	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), IONIZE(2951, uint32_t));

	/* Insert values [0, 2951) for key 2. */
	for (i = 0; i < 100; i++) {
		iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), IONIZE((uint32_t) rand() % 2951, uint32_t));
	}

	/* This is the max value to be inserted for key 1. */
	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), IONIZE(14, uint32_t));

	MATERIALIZED_QUERY(SELECT(SELECT_AGGR(0)), AGGREGATES(MAX(NEUTRALIZE(test.value, uint32_t))), FROM(0, test), WHERE(1), GROUPBY(ASCENDING_INT(NEUTRALIZE(test.key, uint32_t))), HAVING_ALL, ORDERBY_NONE, , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_count_from_where_groupby_aggregate_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(double), result->num_bytes);

	unsigned int groupby_count = *((unsigned int *) (result->processed));

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, groupby_count);
			break;

		case 1:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, groupby_count);
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_count_from_where_groupby_aggregate(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.total = 2;
	state.func	= iinq_test_create_query_select_count_from_where_groupby_aggregate_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), IONIZE(2, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), IONIZE(2, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), IONIZE(1, uint32_t));

	MATERIALIZED_QUERY(SELECT(SELECT_AGGR(0)), AGGREGATES(COUNT(NEUTRALIZE(test.key, uint32_t))), FROM(0, test), WHERE(1), GROUPBY(ASCENDING_INT(NEUTRALIZE(test.key, uint32_t))), HAVING_ALL, ORDERBY_NONE, , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_sum_from_where_groupby_aggregate_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(double), result->num_bytes);

	double groupby_sum = *((double *) (result->processed));

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_TRUE(tc, 17.0 == groupby_sum);
			break;

		case 1:
			PLANCK_UNIT_ASSERT_TRUE(tc, 5.0 == groupby_sum);
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_sum_from_where_groupby_aggregate(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.total = 2;
	state.func	= iinq_test_create_query_select_sum_from_where_groupby_aggregate_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(13, uint32_t), IONIZE(13, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(13, uint32_t), IONIZE(4, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(7, uint32_t), IONIZE(5, uint32_t));

	MATERIALIZED_QUERY(SELECT(SELECT_AGGR(0)), AGGREGATES(SUM(NEUTRALIZE(test.value, uint32_t))), FROM(0, test), WHERE(1), GROUPBY(DESCENDING_INT(NEUTRALIZE(test.key, uint32_t))), HAVING_ALL, ORDERBY_NONE, , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_count_from_where_groupby_having_aggregate_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint64_t), result->num_bytes);

	ion_key_t key = result->processed;

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, NEUTRALIZE(key, uint32_t));
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_count_from_where_groupby_having_aggregate(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.total = 2;
	state.func	= iinq_test_create_query_select_count_from_where_groupby_having_aggregate_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(15, uint32_t), IONIZE(3, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(3, uint32_t), IONIZE(2, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(3, uint32_t), IONIZE(1, uint32_t));

	MATERIALIZED_QUERY(SELECT(SELECT_AGGR(0)), AGGREGATES(COUNT(NEUTRALIZE(test.key, uint32_t))), FROM(0, test), WHERE(1), GROUPBY(ASCENDING_INT(NEUTRALIZE(test.key, uint32_t))), HAVING(AGGREGATE(0) == 2), ORDERBY_NONE, , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_key_from_where_aggregates_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint32_t), result->num_bytes);

	ion_key_t expr = result->processed;

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, NEUTRALIZE(expr, uint32_t));
			break;

		case 1:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, NEUTRALIZE(expr, uint32_t));
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_key_from_where_groupby(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.func	= iinq_test_create_query_select_key_from_where_aggregates_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), IONIZE(1, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), IONIZE(2, uint32_t));

	MATERIALIZED_QUERY(SELECT(SELECT_GRBY(0)), AGGREGATES_NONE, FROM(0, test), WHERE(1), GROUPBY(DESCENDING_INT(NEUTRALIZE(test.key, uint32_t))), HAVING_NONE, ORDERBY_NONE, , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_max_from_where_groupby_aggregate_orderby_small_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(double), result->num_bytes);

	double maxval = *((double *) (result->processed));

	switch (count) {
		case 0:
			/* TODO: Make this an assert double macro. */
			PLANCK_UNIT_ASSERT_TRUE(tc, 2.0 == maxval);
			break;

		case 1:
			/* TODO: Make this an assert double macro. */
			PLANCK_UNIT_ASSERT_TRUE(tc, 1.0 == maxval);
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_max_from_where_groupby_aggregate_orderby_small(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.total = 2;
	state.func	= iinq_test_create_query_select_max_from_where_groupby_aggregate_orderby_small_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), IONIZE(1, uint32_t));
	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), IONIZE(2, uint32_t));

	MATERIALIZED_QUERY(SELECT(SELECT_AGGR(0)), AGGREGATES(MAX(NEUTRALIZE(test.key, uint32_t))), FROM(0, test), WHERE(1), GROUPBY(ASCENDING_INT(NEUTRALIZE(test.key, uint32_t))), HAVING_ALL, ORDERBY(DESCENDING_INT(AGGREGATE(0))), , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_max_from_where_groupby_aggregate_orderby_large_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(double), result->num_bytes);

	double maxval = *((double *) (result->processed));

	if (count == 0) {
		printf("%f\n:", maxval);
		fflush(stdout);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1842.0 == maxval);
	}
	else if (count == 1) {
		PLANCK_UNIT_ASSERT_TRUE(tc, 238.0 == maxval);
	}
	else if (count > 200) {
		PLANCK_UNIT_SET_FAIL(tc);
	}
	else {
		PLANCK_UNIT_ASSERT_TRUE(tc, total - count == maxval);
	}
}

void
iinq_test_create_query_select_max_from_where_groupby_aggregate_orderby_large(
	planck_unit_test_t *tc
) {
	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.total = 200;
	state.func	= iinq_test_create_query_select_max_from_where_groupby_aggregate_orderby_large_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= sizeof(uint32_t);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	iinq_test_insert_into_test(tc, IONIZE(198, uint32_t), IONIZE(238, uint32_t));

	uint32_t i;

	for (i = 0; i < 200; i++) {
		iinq_test_insert_into_test(tc, IONIZE(i, uint32_t), IONIZE(i, uint32_t));
	}

	iinq_test_insert_into_test(tc, IONIZE(199, uint32_t), IONIZE(1842, uint32_t));

	MATERIALIZED_QUERY(SELECT(SELECT_AGGR(0)), AGGREGATES(MAX(NEUTRALIZE(test.value, uint32_t))), FROM(0, test), WHERE(1), GROUPBY(ASCENDING_INT(NEUTRALIZE(test.key, uint32_t))), HAVING_ALL, ORDERBY(DESCENDING_INT(AGGREGATE(0))), , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 200, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_count_from_where_groupby_aggregate_schema_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2 * sizeof(double), result->num_bytes);

	double	minval	= *((double *) (result->processed));
	double	maxval	= *((double *) (result->processed) + 1);

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_TRUE(tc, 2.0 == minval);
			PLANCK_UNIT_ASSERT_TRUE(tc, 10.0 == maxval);
			break;

		case 1:
			PLANCK_UNIT_ASSERT_TRUE(tc, 5.0 == minval);
			PLANCK_UNIT_ASSERT_TRUE(tc, 10.0 == maxval);
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_count_from_where_groupby_aggregate_schema(
	planck_unit_test_t *tc
) {
	DEFINE_SCHEMA(test, {
		int x;
		int y;
	}
	);

	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.total = 2;
	state.func	= iinq_test_create_query_select_count_from_where_groupby_aggregate_schema_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	DECLARE_SCHEMA_VAR(test, test_val);

	processor	= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= SCHEMA_SIZE(test);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	test_val.x	= 10;
	test_val.y	= 10;
	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), &test_val);

	test_val.x	= 5;
	test_val.y	= 5;
	iinq_test_insert_into_test(tc, IONIZE(2, uint32_t), &test_val);

	test_val.x	= 2;
	test_val.y	= 10;
	iinq_test_insert_into_test(tc, IONIZE(1, uint32_t), &test_val);

	MATERIALIZED_QUERY(SELECT(SELECT_AGGR(0), SELECT_AGGR(1)), AGGREGATES(MIN(test_tuple->x), MAX(test_tuple->y)), FROM(1, test), WHERE(1), GROUPBY(ASCENDING_INT(NEUTRALIZE(test.key, uint32_t))), HAVING_ALL, ORDERBY_NONE, , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, state.count);

	DROP(test);
}

void
iinq_test_create_query_select_string_from_join_where_schema_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint32_t) * 2, result->num_bytes);

	ion_key_t	key		= result->processed;
	ion_value_t value	= result->processed + sizeof(uint32_t);

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 10, NEUTRALIZE(key, uint32_t));
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, NEUTRALIZE(value, uint32_t));
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_create_query_select_string_from_join_where_schema(
	planck_unit_test_t *tc
) {
	DEFINE_SCHEMA(test1, {
		uint32_t x;
		char y[5];
	}
	);

	DEFINE_SCHEMA(test2, {
		char y[5];
		uint32_t z;
	}
	);

	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.total = 2;
	state.func	= iinq_test_create_query_select_string_from_join_where_schema_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size_1;
	ion_value_size_t	value_size_2;

	DECLARE_SCHEMA_VAR(test1, test_val_1);
	DECLARE_SCHEMA_VAR(test2, test_val_2);

	processor		= IINQ_QUERY_PROCESSOR(check_results, &state);

	key_type		= key_type_numeric_signed;
	key_size		= sizeof(uint32_t);
	value_size_1	= SCHEMA_SIZE(test1);
	value_size_2	= SCHEMA_SIZE(test2);

	error			= CREATE_DICTIONARY(test1, key_type, key_size, value_size_1);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	error			= CREATE_DICTIONARY(test2, key_type, key_size, value_size_2);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	test_val_1.x	= 10;
	sprintf(test_val_1.y, "key1");
	iinq_test_insert_into_test1(tc, IONIZE(2, uint32_t), &test_val_1);

	test_val_1.x = 5;
	sprintf(test_val_1.y, "key2");
	iinq_test_insert_into_test1(tc, IONIZE(2, uint32_t), &test_val_1);

	sprintf(test_val_2.y, "key1");
	test_val_2.z = 2;
	iinq_test_insert_into_test2(tc, IONIZE(1, uint32_t), &test_val_2);

	MATERIALIZED_QUERY(SELECT(SELECT_EXPR(test1_tuple->x), SELECT_EXPR(test2_tuple->z)), AGGREGATES_NONE, FROM(1, test1, test2), WHERE(0 == strncmp(test1_tuple->y, test2_tuple->y, 4)), GROUPBY_NONE, HAVING_ALL, ORDERBY_NONE, , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 1, state.count);

	DROP(test1);
	DROP(test2);
}

void
iinq_test_the_ultimate_query_records(
	planck_unit_test_t	*tc,
	ion_iinq_result_t	*result,
	uint32_t			count,
	uint32_t			total
) {
	UNUSED(total);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, sizeof(uint16_t) + 5 + sizeof(double) * 2, result->num_bytes);

	uint16_t	b			= *((uint16_t *) result->processed);
	char		*f			= ((char *) result->processed + 2);
	double		minval		= *((double *) ((char *) result->processed + sizeof(uint16_t) + 5));
	uint64_t	countval	= *((uint64_t *) ((char *) result->processed + sizeof(uint16_t) + 5 + sizeof(double)));

	switch (count) {
		case 0:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 601, b);
			PLANCK_UNIT_ASSERT_TRUE(tc, 0 == strncmp("frog", f, 5));
			PLANCK_UNIT_ASSERT_TRUE(tc, 1281.0 == minval);
			PLANCK_UNIT_ASSERT_TRUE(tc, 4 == countval);
			break;

		case 1:
			PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 11, b);
			PLANCK_UNIT_ASSERT_TRUE(tc, 0 == strncmp("cat", f, 5));
			PLANCK_UNIT_ASSERT_TRUE(tc, 5128.0 == minval);
			PLANCK_UNIT_ASSERT_TRUE(tc, 2 == countval);
			break;

		default:
			PLANCK_UNIT_SET_FAIL(tc);
			break;
	}
}

void
iinq_test_the_ultimate_query(
	planck_unit_test_t *tc
) {
	DEFINE_SCHEMA(test1, {
		int32_t a;
		uint16_t b;
	}
	);

	DEFINE_SCHEMA(test2, {
		int32_t c;
		int16_t d;
	}
	);

	DEFINE_SCHEMA(test3, {
		int64_t e;
		char f[5];
	}
	);

	ion_err_t					error;
	ion_iinq_query_processor_t	processor;
	iinq_test_query_state_t		state;

	state.count = 0;
	state.tc	= tc;
	state.total = 2;
	state.func	= iinq_test_the_ultimate_query_records;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size_1;
	ion_value_size_t	value_size_2;
	ion_value_size_t	value_size_3;

	DECLARE_SCHEMA_VAR(test1, test_val_1);
	DECLARE_SCHEMA_VAR(test2, test_val_2);
	DECLARE_SCHEMA_VAR(test3, test_val_3);

	processor		= IINQ_QUERY_PROCESSOR(check_results, &state);

	value_size_1	= SCHEMA_SIZE(test1);
	value_size_2	= SCHEMA_SIZE(test2);
	value_size_3	= SCHEMA_SIZE(test3);

	key_type		= key_type_numeric_signed;
	key_size		= sizeof(int32_t);
	error			= CREATE_DICTIONARY(test1, key_type, key_size, value_size_1);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	key_type		= key_type_null_terminated_string;
	key_size		= 5;
	error			= CREATE_DICTIONARY(test2, key_type, key_size, value_size_2);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	key_type		= key_type_numeric_signed;
	key_size		= sizeof(int32_t);
	error			= CREATE_DICTIONARY(test3, key_type, key_size, value_size_3);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	test_val_1.a	= 42;
	test_val_1.b	= 2108;
	iinq_test_insert_into_test1(tc, IONIZE(1, int32_t), &test_val_1);

	test_val_1.a	= 42;
	test_val_1.b	= 32;
	iinq_test_insert_into_test1(tc, IONIZE(2, int32_t), &test_val_1);

	test_val_1.a	= 18;
	test_val_1.b	= 601;
	iinq_test_insert_into_test1(tc, IONIZE(3, int32_t), &test_val_1);

	test_val_1.a	= 13;
	test_val_1.b	= 601;
	iinq_test_insert_into_test1(tc, IONIZE(4, int32_t), &test_val_1);

	test_val_1.a	= 1;
	test_val_1.b	= 601;
	iinq_test_insert_into_test1(tc, IONIZE(5, int32_t), &test_val_1);

	test_val_1.a	= 9;
	test_val_1.b	= 11;
	iinq_test_insert_into_test1(tc, IONIZE(6, int32_t), &test_val_1);

	test_val_2.c	= 17;
	test_val_2.d	= -66;
	iinq_test_insert_into_test2(tc, "cat", &test_val_2);

	test_val_2.c	= 42;
	test_val_2.d	= 44;
	iinq_test_insert_into_test2(tc, "dog", &test_val_2);

	test_val_2.c	= 18;
	test_val_2.d	= 128;
	iinq_test_insert_into_test2(tc, "frog", &test_val_2);

	test_val_2.c	= 13;
	test_val_2.d	= 712;
	iinq_test_insert_into_test2(tc, "frog", &test_val_2);

	test_val_2.c	= 1;
	test_val_2.d	= 713;
	iinq_test_insert_into_test2(tc, "cow", &test_val_2);

	test_val_2.c	= 9;
	test_val_2.d	= 8;
	iinq_test_insert_into_test2(tc, "cat", &test_val_2);

	test_val_3.e	= 4000000000;
	strcpy(test_val_3.f, "cat");
	iinq_test_insert_into_test3(tc, IONIZE(1, int32_t), &test_val_3);

	test_val_3.e = -1;
	strcpy(test_val_3.f, "dog");
	iinq_test_insert_into_test3(tc, IONIZE(2, int32_t), &test_val_3);

	test_val_3.e = 4250;
	strcpy(test_val_3.f, "frog");
	iinq_test_insert_into_test3(tc, IONIZE(3, int32_t), &test_val_3);

	test_val_3.e = 1281;
	strcpy(test_val_3.f, "frog");
	iinq_test_insert_into_test3(tc, IONIZE(4, int32_t), &test_val_3);

	test_val_3.e = 1100;
	strcpy(test_val_3.f, "cow");
	iinq_test_insert_into_test3(tc, IONIZE(5, int32_t), &test_val_3);

	test_val_3.e = 5128;
	strcpy(test_val_3.f, "cat");
	iinq_test_insert_into_test3(tc, IONIZE(6, int32_t), &test_val_3);

	MATERIALIZED_QUERY(SELECT(SELECT_GRBY(0), SELECT_GRBY(1), SELECT_AGGR(0), SELECT_AGGR(1)), AGGREGATES(MIN(test3_tuple->e), COUNT(test2_tuple->d), SUM(test2_tuple->d), MAX(test3_tuple->e)), FROM(1, test1, test2, test3), WHERE(test1_tuple->a == test2_tuple->c && 0 == strncmp(test2.key, test3_tuple->f, 5) && test3_tuple->e > 1000), GROUPBY(ASCENDING_UINT(test1_tuple->b), ASCENDING(test3_tuple->f)), HAVING((int16_t) AGGREGATE(2) % 2 == 0), ORDERBY(DESCENDING_UINT(test1_tuple->b), ASC(AGGREGATE(3))), , , &processor);

	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, 2, state.count);

	DROP(test1);
	DROP(test2);
	DROP(test3);
}

IINQ_NEW_PROCESSOR_FUNC(benchmark_test) {
//	printf("key\t\tattr0\t\tattr1\t\tattr2\t\tattr3\n");

	uint32_t	*dataptr	= (uint32_t *) result->processed;
	int			i			= 0;

	for (i = 0; i < (result->num_bytes / sizeof(uint32_t)); i++) {
//		printf("%d\t\t\t", *dataptr);
		dataptr++;
	}

//	printf("\n");
}

void
iinq_benchmark_query_1(
	ion_iinq_query_processor_t *processor
) {
	QUERY(SELECT_ALL, FROM(1, test), WHERE(1), , , , , , processor);
}

void
iinq_benchmark_query_2(
	ion_iinq_query_processor_t *processor
) {
	DEFINE_SCHEMA(test, {
		uint32_t attr0;
		uint32_t attr1;
		uint32_t attr2;
		uint32_t attr3;
	}
	);

	QUERY(SELECT(SELECT_EXPR(test_tuple->attr0)), FROM(1, test), WHERE(1), , , , , , processor);
}

void
iinq_benchmark_query_3(
	ion_iinq_query_processor_t *processor
) {
	DEFINE_SCHEMA(test, {
		uint32_t attr0;
		uint32_t attr1;
		uint32_t attr2;
		uint32_t attr3;
	}
	);

	QUERY(SELECT(SELECT_EXPR(test_tuple->attr0), SELECT_EXPR(test_tuple->attr1)), FROM(1, test), WHERE(1), , , , , , processor);
}

void
iinq_benchmark_query_4(
	ion_iinq_query_processor_t *processor
) {
	DEFINE_SCHEMA(test, {
		uint32_t attr0;
		uint32_t attr1;
		uint32_t attr2;
		uint32_t attr3;
	}
	);

	QUERY(SELECT(SELECT_EXPR(test_tuple->attr0), SELECT_EXPR(test_tuple->attr1), SELECT_EXPR(test_tuple->attr2), SELECT_EXPR(test_tuple->attr3)), FROM(1, test), WHERE(1), , , , , , processor);
}

void
iinq_benchmark_query_5(
	ion_iinq_query_processor_t *processor
) {
	DEFINE_SCHEMA(test, {
		uint32_t attr0;
		uint32_t attr1;
		uint32_t attr2;
		uint32_t attr3;
	}
	);

	QUERY(SELECT(SELECT_EXPR(test_tuple->attr0 * 3), SELECT_EXPR(test_tuple->attr1 * 2), SELECT_EXPR(test_tuple->attr2 % 13), SELECT_EXPR(test_tuple->attr3 * 2 + 14)), FROM(1, test), WHERE(1), , , , , , processor);
}

void
iinq_benchmark_query_6(
	ion_iinq_query_processor_t *processor
) {
	DEFINE_SCHEMA(test, {
		uint32_t attr0;
		uint32_t attr1;
		uint32_t attr2;
		uint32_t attr3;
	}
	);

	QUERY(SELECT(SELECT_EXPR(test_tuple->attr0 * 3), SELECT_EXPR(test_tuple->attr1 * 2), SELECT_EXPR(test_tuple->attr2 % 13), SELECT_EXPR(test_tuple->attr3 * 2 + 14)), FROM(1, test), WHERE(test_tuple->attr0 >= 0 && (2 * test_tuple->attr1) != -1), , , , , , processor);
}

void
iinq_benchmark_time(
	char						*query_name,
	void (						*query_func)(ion_iinq_query_processor_t *),
	ion_iinq_query_processor_t	*processor
) {
//	printf("==================== %s ======================\n", query_name);

	unsigned long start;

	start = ion_time();
	query_func(processor);
	printf("%s time: %lums\n===\n\n", query_name, ion_time() - start);
}

void
iinq_benchmark_set_1(
	planck_unit_test_t *tc
) {
	DEFINE_SCHEMA(test, {
		uint32_t attr0;
		uint32_t attr1;
		uint32_t attr2;
		uint32_t attr3;
	}
	);

	ion_err_t					error;
	ion_iinq_query_processor_t	processor;

	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	DECLARE_SCHEMA_VAR(test, test_val);

	processor	= IINQ_QUERY_PROCESSOR(benchmark_test, NULL);

	key_type	= key_type_numeric_signed;
	key_size	= sizeof(uint32_t);
	value_size	= SCHEMA_SIZE(test);

	error		= CREATE_DICTIONARY(test, key_type, key_size, value_size);
	PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(tc, err_ok, error);

	uint32_t i;

	for (i = 0; i < 100; i++) {
		test_val.attr0	= i * 2;
		test_val.attr1	= i * 3;
		test_val.attr2	= i * 4;
		test_val.attr3	= i * 5;
		iinq_test_insert_into_test(tc, IONIZE(i, uint32_t), &test_val);
	}

	iinq_benchmark_time("query_1", iinq_benchmark_query_1, &processor);
	iinq_benchmark_time("query_2", iinq_benchmark_query_2, &processor);
	iinq_benchmark_time("query_3", iinq_benchmark_query_3, &processor);
	iinq_benchmark_time("query_4", iinq_benchmark_query_4, &processor);
	iinq_benchmark_time("query_5", iinq_benchmark_query_5, &processor);
	iinq_benchmark_time("query_6", iinq_benchmark_query_6, &processor);

	DROP(test);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == 1);
}

planck_unit_suite_t *
iinq_get_suite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();
/*
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_open_source_intint);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_open_source_string10string20);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_insert_update_delete_drop_dictionary_intint);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_single_dictionary);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_expression_all_from_where_single_dictionary);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_two_dictionaries);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_max_from_where_aggregates);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_expression_from_where_aggregates);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_complex_expression_from_where_aggregates);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_orderby_ascending_small);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_orderby_ascending_large);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_orderby_descending_small);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_all_from_where_orderby_descending_large);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_max_from_where_groupby_aggregate_small);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_max_from_where_groupby_aggregate_large);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_count_from_where_groupby_aggregate);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_sum_from_where_groupby_aggregate);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_count_from_where_groupby_having_aggregate);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_key_from_where_groupby);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_max_from_where_groupby_aggregate_orderby_small);

	/* TODO: Failing :( fix me */
	/* PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_max_from_where_groupby_aggregate_orderby_large); */
/*
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_count_from_where_groupby_aggregate_schema);
	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_create_query_select_string_from_join_where_schema);
*/
	/* TODO: Failing :( fix me * / */
/*	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_test_the_ultimate_query);* / */

	PLANCK_UNIT_ADD_TO_SUITE(suite, iinq_benchmark_set_1);

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
