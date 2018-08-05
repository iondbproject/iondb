#include "iinq_testing_functions.h"

size_t
iinq_calculate_offset(
	iinq_table_id_t		table_id,
	iinq_field_num_t	field_num
) {
	switch (table_id) {
		case 0: {
			switch (field_num) {
				case 1:
					return IINQ_BITS_FOR_NULL(3);

				case 2:
					return IINQ_BITS_FOR_NULL(3) + sizeof(int);

				case 3:
					return IINQ_BITS_FOR_NULL(3) + sizeof(int) + (sizeof(char) * 31);

				case 4:
					return IINQ_BITS_FOR_NULL(3) + sizeof(int) + (sizeof(char) * 31) + sizeof(int);

				default:
					return 0;
			}
		}

		default:
			return 0;
	}
}

iinq_query_operator_t *
iinq_selection_init(
	iinq_query_operator_t	*input_operator,
	unsigned int			num_conditions,
	iinq_where_params_t		*conditions
) {
	if (NULL == input_operator) {
		return NULL;
	}

	iinq_query_operator_t *operator = malloc(sizeof(iinq_query_operator_t));

	if (NULL == operator) {
		input_operator->destroy(&input_operator);
		return NULL;
	}

	operator->instance = malloc(sizeof(iinq_selection_t));

	if (NULL == operator->instance) {
		free(operator);
		input_operator->destroy(&input_operator);
		return NULL;
	}

	iinq_selection_t *selection = (iinq_selection_t *) operator->instance;

	selection->super.input_operators = malloc(sizeof(iinq_query_operator_t *));

	if (NULL == selection->super.input_operators) {
		free(selection);
		free(operator);
		input_operator->destroy(&input_operator);
		return NULL;
	}

	selection->super.type					= iinq_selection_e;
	selection->super.num_input_operators	= 1;

	selection->super.input_operators[0]		= input_operator;
	selection->super.num_fields				= input_operator->instance->num_fields;
	selection->super.null_indicators		= input_operator->instance->null_indicators;
	selection->super.field_info				= input_operator->instance->field_info;
	selection->super.fields					= input_operator->instance->fields;

	selection->num_conditions				= num_conditions;
	selection->conditions					= malloc(sizeof(iinq_where_params_t) * num_conditions);

	if (NULL == selection->conditions) {
		free(selection->super.input_operators);
		free(selection);
		free(operator);
		input_operator->destroy(&input_operator);
		return NULL;
	}

	int i;

	for (i = 0; i < num_conditions; i++) {
		iinq_table_id_t		table_id	= input_operator->instance->field_info[conditions[i].where_field - 1].table_id;
		iinq_field_num_t	field_num	= input_operator->instance->field_info[conditions[i].where_field - 1].field_num;

		selection->conditions[i].where_field = conditions[i].where_field;

		size_t value_size = iinq_calculate_offset(table_id, field_num + 1) - iinq_calculate_offset(table_id, field_num);

		selection->conditions[i].field_value = malloc(value_size);

		if (NULL == selection->conditions[i].field_value) {
			int j;

			for (j = 0; j < i; j++) {
				free(selection->conditions[i].field_value);
			}

			free(selection->conditions);
			free(selection->super.input_operators);
			free(selection);
			free(operator);
			input_operator->destroy(&input_operator);
			return NULL;
		}

		selection->conditions[i].bool_operator = conditions[i].bool_operator;

		switch (iinq_get_field_type(table_id, field_num)) {
			case iinq_int:
				*(int *) selection->conditions[i].field_value = NEUTRALIZE(conditions[i].field_value, int);
				break;

			case iinq_null_terminated_string:
				strncpy((char *) selection->conditions[i].field_value, (char *) conditions[i].field_value, value_size);
				break;

			default:
				memcpy(selection->conditions[i].field_value, conditions[i].field_value, value_size);
				break;
		}
	}

	operator->next		= iinq_selection_next;
	operator->destroy	= iinq_selection_destroy;

	return operator;
}

ion_boolean_t
iinq_projection_next(
	iinq_query_operator_t *operator
) {
	ion_boolean_t result = operator->instance->input_operators[0]->next(operator->instance->input_operators[0]);

	if (result) {
		int					i;
		iinq_projection_t	*projection = (iinq_projection_t *) operator->instance;

		for (i = 0; i < projection->super.num_fields; i++) {
			if (iinq_check_null_indicator(projection->super.input_operators[0]->instance->null_indicators, projection->input_field_nums[i])) {
				iinq_set_null_indicator(projection->super.null_indicators, i + 1);
			}
			else {
				iinq_clear_null_indicator(projection->super.null_indicators, i + 1);
			}
		}

		operator->status.count++;
	}

	return result;
}

ion_err_t
iinq_execute_prepared(
	iinq_prepared_sql *p
) {
	switch (p->table) {
		case 0: {
			if (iinq_check_null_indicator(p->value, 1)) {
				return err_unable_to_insert;
			}

			return iinq_execute(0, p->key, p->value, iinq_insert_t);
		}
	}
}

size_t
iinq_calculate_key_offset(
	iinq_table_id_t		table_id,
	iinq_field_num_t	field_num
) {
	switch (table_id) {
		case 0:

			switch (field_num) {
				case 1:
					return 0;
			}
	}
}

iinq_query_operator_t *
iinq_projection_init(
	iinq_query_operator_t	*input_operator,
	iinq_field_num_t		num_fields,
	iinq_field_num_t		*field_nums
) {
	if (NULL == input_operator) {
		return NULL;
	}

	iinq_query_operator_t *operator = malloc(sizeof(iinq_query_operator_t));

	if (NULL == operator) {
		input_operator->destroy(&input_operator);
		return NULL;
	}

	operator->instance = malloc(sizeof(iinq_projection_t));

	if (NULL == operator->instance) {
		free(operator);
		input_operator->destroy(&input_operator);
		return NULL;
	}

	operator->next		= iinq_projection_next;
	operator->destroy	= iinq_projection_destroy;

	iinq_projection_t *projection = (iinq_projection_t *) operator->instance;

	projection->super.type					= iinq_projection_e;

	projection->super.num_input_operators	= 1;
	projection->super.input_operators		= malloc(sizeof(iinq_query_operator_t));

	if (NULL == projection->super.input_operators) {
		free(projection);
		free(operator);
		input_operator->destroy(&input_operator);
		return NULL;
	}

	projection->super.input_operators[0]	= input_operator;
	projection->super.num_fields			= num_fields;

	projection->input_field_nums			= malloc(sizeof(iinq_field_num_t) * num_fields);

	if (NULL == projection->input_field_nums) {
		free(projection->super.input_operators);
		free(projection);
		free(operator);
		input_operator->destroy(&input_operator);
		return NULL;
	}

	memcpy(projection->input_field_nums, field_nums, sizeof(iinq_field_num_t) * num_fields);

	projection->super.null_indicators = malloc(IINQ_BITS_FOR_NULL(num_fields));

	if (NULL == projection->super.null_indicators) {
		free(projection->input_field_nums);
		free(projection->super.input_operators);
		free(projection);
		free(operator);
		input_operator->destroy(&input_operator);
		return NULL;
	}

	projection->super.field_info = malloc(sizeof(iinq_field_info_t) * num_fields);

	if (NULL == projection->super.field_info) {
		free(projection->super.null_indicators);
		free(projection->super.input_operators);
		free(projection);
		free(operator);
		input_operator->destroy(&input_operator);
		return NULL;
	}

	projection->super.fields = malloc(sizeof(ion_value_t) * num_fields);

	if (NULL == projection->super.fields) {
		free(projection->super.field_info);
		free(projection->super.null_indicators);
		free(projection->super.input_operators);
		free(projection);
		free(operator);
		input_operator->destroy(&input_operator);
		return NULL;
	}

	int i;

	for (i = 0; i < num_fields; i++) {
		projection->super.field_info[i] = input_operator->instance->field_info[field_nums[i] - 1];
		projection->super.fields[i]		= input_operator->instance->fields[field_nums[i] - 1];
	}

	operator->status = ION_STATUS_OK(0);

	return operator;
}

void
iinq_projection_destroy(
	iinq_query_operator_t **operator
) {
	if (*operator != NULL) {
		if ((*operator)->instance != NULL) {
			iinq_projection_t *projection = (iinq_projection_t *) (*operator)->instance;

			if (NULL != projection->input_field_nums) {
				free(projection->input_field_nums);
			}

			if (NULL != projection->super.fields) {
				free(projection->super.fields);
			}

			if (NULL != projection->super.field_info) {
				free(projection->super.field_info);
			}

			if (NULL != projection->super.null_indicators) {
				free(projection->super.null_indicators);
			}

			if (NULL != projection->super.input_operators) {
				projection->super.input_operators[0]->destroy(&projection->super.input_operators[0]);
				free(projection->super.input_operators);
			}

			free(projection);
		}

		free(*operator);
		*operator = NULL;
	}
}

ion_boolean_t
iinq_table_scan_next(
	iinq_query_operator_t *operator
) {
	iinq_table_scan_t *table_scan = (iinq_table_scan_t *) operator->instance;

	if ((cs_cursor_active == table_scan->cursor->next(table_scan->cursor, &table_scan->record)) || (cs_cursor_initialized == table_scan->cursor->status)) {
		operator->status.count++;
		return boolean_true;
	}

	return boolean_false;
}

ion_err_t
iinq_print_keys(
	iinq_table_id_t table_id
) {
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;
	ion_err_t					error;
	ion_predicate_t				predicate;
	ion_dict_cursor_t			*cursor = NULL;
	ion_cursor_status_t			cursor_status;
	ion_record_t				ion_record;

	ion_record.key		= NULL;
	ion_record.value	= NULL;

	error				= iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != error) {
		return error;
	}

	error = dictionary_build_predicate(&predicate, predicate_all_records);

	if (err_ok != error) {
		ion_close_dictionary(&dictionary);
		return error;
	}

	error = dictionary_find(&dictionary, &predicate, &cursor);

	if (err_ok != error) {
		ion_close_dictionary(&dictionary);
		return error;
	}

	unsigned char *key;

	switch (table_id) {
		case 0:
			ion_record.key = malloc(sizeof(int));

			if (NULL == ion_record.key) {
				error = err_out_of_memory;
				goto END;
			}

			ion_record.value = malloc(IINQ_BITS_FOR_NULL(3) + (sizeof(int) * 2) + (sizeof(char) * 31));

			if (NULL == ion_record.value) {
				error = err_out_of_memory;
				goto END;
			}

			while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				key = ion_record.key;

				printf("%10d\n", NEUTRALIZE(key, int));
			}

			break;
	}

	printf("\n");
END:

	if (NULL != cursor) {
		cursor->destroy(&cursor);
	}

	if (NULL != ion_record.key) {
		free(ion_record.key);
	}

	;

	if (NULL != ion_record.value) {
		free(ion_record.value);
	}

	;

	ion_close_dictionary(&dictionary);

	return error;
}

ion_err_t
drop_table(
	iinq_table_id_t table_id
) {
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;
	ion_err_t					error;

	error = iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != error) {
		return error;
	}

	ion_close_dictionary(&dictionary);
	error = iinq_drop(table_id);
	return error;
}

iinq_query_operator_t *
iinq_table_scan_init(
	iinq_table_id_t		table_id,
	iinq_field_num_t	num_fields
) {
	int							i;
	ion_err_t					error;
	iinq_table_scan_t			*table_scan;
	ion_predicate_t				*predicate;
	ion_dict_cursor_t			*cursor		= NULL;
	ion_record_t				*record		= NULL;
	ion_dictionary_t			*dictionary = NULL;
	ion_dictionary_handler_t	*handler	= NULL;

	iinq_query_operator_t *operator			= malloc(sizeof(iinq_query_operator_t));

	if (NULL == operator) {
		return NULL;
	}

	operator->instance = malloc(sizeof(iinq_table_scan_t));

	if (NULL == operator->instance) {
		free(operator);
		return NULL;
	}

	table_scan						= (iinq_table_scan_t *) operator->instance;
	table_scan->super.type			= iinq_table_scan_e;
	table_scan->super.num_fields	= num_fields;
	predicate						= &table_scan->predicate;
	error							= dictionary_build_predicate(predicate, predicate_all_records);

	if (err_ok != error) {
		free(operator->instance);
		free(operator);
		return NULL;
	}

	dictionary			= &table_scan->dictionary;
	handler				= &table_scan->handler;
	dictionary->handler = handler;
	record				= &table_scan->record;

	error				= iinq_open_source(table_id, dictionary, handler);

	if (err_ok != error) {
		free(operator->instance);
		free(operator);
		return NULL;
	}

	error = dictionary_find(dictionary, predicate, &cursor);

	if (err_ok != error) {
		if (NULL != cursor) {
			cursor->destroy(&cursor);
		}

		ion_close_dictionary(&dictionary);
		free(operator->instance);
		free(operator);
		return NULL;
	}

	table_scan->cursor		= cursor;

	table_scan->record.key	= malloc(dictionary->instance->record.key_size);

	if (NULL == table_scan->record.key) {
		cursor->destroy(&cursor);
		ion_close_dictionary(&dictionary);
		free(operator->instance);
		free(operator);
		return NULL;
	}

	record->value = malloc(dictionary->instance->record.value_size);

	if (NULL == record->value) {
		free(record->key);
		cursor->destroy(&cursor);
		ion_close_dictionary(&dictionary);
		free(operator->instance);
		free(operator);
		return NULL;
	}

	table_scan->super.field_info = malloc(sizeof(iinq_field_info_t) * num_fields);

	if (NULL == table_scan->super.field_info) {
		free(record->value);
		free(record->key);
		cursor->destroy(&cursor);
		ion_close_dictionary(&dictionary);
		free(operator->instance);
		free(operator);
		return NULL;
	}

	table_scan->super.null_indicators	= table_scan->record.value;

	table_scan->super.fields			= malloc(sizeof(ion_value_t) * num_fields);

	if (NULL == table_scan->super.fields) {
		free(table_scan->super.field_info);
		free(record->value);
		free(record->key);
		cursor->destroy(&cursor);
		ion_close_dictionary(&dictionary);
		free(operator->instance);
		free(operator);
		return NULL;
	}

	for (i = 0; i < num_fields; i++) {
		table_scan->super.field_info[i] = (iinq_field_info_t) {
			table_id, i + 1
		};
		table_scan->super.fields[i]		= (unsigned char *) record->value + iinq_calculate_offset(table_id, i + 1);
	}

	ion_close_master_table();
	operator->next		= iinq_table_scan_next;
	operator->destroy	= iinq_table_scan_destroy;
	operator->status	= ION_STATUS_OK(0);

	return operator;
}

void
iinq_set_param(
	iinq_prepared_sql	*p,
	iinq_field_num_t	field_num,
	ion_value_t			val
) {
	iinq_null_indicator_t	*null_indicators	= p->value;
	unsigned char			*data				= ((char *) p->value) + iinq_calculate_offset(p->table, field_num);

	if (NULL == val) {
		iinq_set_null_indicator(null_indicators, field_num);
	}
	else {
		iinq_clear_null_indicator(null_indicators, field_num);

		iinq_field_t type = iinq_get_field_type(p->table, field_num);

		if (type == iinq_int) {
			if (iinq_is_key_field(p->table, field_num)) {
				*(int *) (p->key + iinq_calculate_key_offset(p->table, field_num)) = NEUTRALIZE(val, int);
			}

			*(int *) data = NEUTRALIZE(val, int);
		}
		else {
			size_t size = iinq_calculate_offset(p->table, field_num + 1) - iinq_calculate_offset(p->table, field_num);

			if (iinq_is_key_field(p->table, field_num)) {
				strncpy(p->key + iinq_calculate_key_offset(p->table, field_num), val, size);
			}

			strncpy(data, val, size);
		}
	}
}

ion_boolean_t
iinq_is_key_field(
	iinq_table_id_t		table_id,
	iinq_field_num_t	field_num
) {
	switch (table_id) {
		case 0:

			switch (field_num) {
				case 1:
					return boolean_true;

				default:
					return boolean_false;
			}

		default:
			return boolean_false;
	}
}

void
iinq_table_scan_destroy(
	iinq_query_operator_t **operator
) {
	if (NULL != *operator) {
		if (NULL != (*operator)->instance) {
			iinq_table_scan_t *table_scan = (iinq_table_scan_t *) (*operator)->instance;

			if (NULL != table_scan->super.field_info) {
				free(table_scan->super.field_info);
			}

			if (NULL != table_scan->super.fields) {
				free(table_scan->super.fields);
			}

			if (NULL != table_scan->record.value) {
				free(table_scan->record.value);
			}

			if (NULL != table_scan->record.key) {
				free(table_scan->record.key);
			}

			if (NULL != table_scan->cursor) {
				table_scan->cursor->destroy(&table_scan->cursor);
			}

			ion_close_dictionary(&table_scan->dictionary);

			free(table_scan);
		}

		free(*operator);
		*operator = NULL;
	}
}

void
iinq_selection_destroy(
	iinq_query_operator_t **operator
) {
	if (NULL != *operator) {
		if (NULL != (*operator)->instance) {
			iinq_selection_t *selection = (iinq_selection_t *) (*operator)->instance;

			if (NULL != selection->conditions) {
				int i;

				for (i = 0; i < selection->num_conditions; i++) {
					if (NULL != selection->conditions[i].field_value) {
						free(selection->conditions[i].field_value);
					}
				}

				free(selection->conditions);
			}

			if (NULL != selection->super.input_operators) {
				selection->super.input_operators[0]->destroy(&selection->super.input_operators[0]);
				free(selection->super.input_operators);
			}

			free(selection);
		}

		free(*operator);
		*operator = NULL;
	}
}

iinq_field_t
iinq_get_field_type(
	iinq_table_id_t		table,
	iinq_field_num_t	field_num
) {
	switch (table) {
		case 0: {
			switch (field_num) {
				case 1:
					return iinq_int;

				case 2:
					return iinq_null_terminated_string;

				case 3:
					return iinq_int;

				default:
					return 0;
			}
		}

		default:
			return 0;
	}
}

ion_err_t
iinq_print_table(
	iinq_table_id_t table_id
) {
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	ion_cursor_status_t cursor_status;

	ion_record_t ion_record;

	ion_record.key		= NULL;
	ion_record.value	= NULL;

	ion_dict_cursor_t *cursor = NULL;

	dictionary.handler = &handler;

	ion_err_t error = iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != error) {
		return error;
	}

	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	dictionary_find(&dictionary, &predicate, &cursor);

	switch (table_id) {
		case 0:
			ion_record.key = malloc(sizeof(int));

			if (NULL == ion_record.key) {
				error = err_out_of_memory;
				goto END;
			}

			ion_record.value = malloc(IINQ_BITS_FOR_NULL(3) + (sizeof(int) * 2) + (sizeof(char) * 31));

			if (NULL == ion_record.value) {
				error = err_out_of_memory;
				goto END;
			}

			break;
	}

	unsigned char			*value;
	iinq_null_indicator_t	*null_indicators;

	switch (table_id) {
		case 0:
			null_indicators = ion_record.value;

			while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				value = ((unsigned char *) ion_record.value) + IINQ_BITS_FOR_NULL(3);

				if (!iinq_check_null_indicator(null_indicators, 1)) {
					printf("%10d, ", NEUTRALIZE(value, int));
				}
				else {
					printf("NULL, ");
				}

				value += sizeof(int);

				if (!iinq_check_null_indicator(null_indicators, 2)) {
					printf("%31s, ", (char *) value);
				}
				else {
					printf("NULL, ");
				}

				value += (sizeof(char) * 31);

				if (!iinq_check_null_indicator(null_indicators, 3)) {
					printf("%10d\n", NEUTRALIZE(value, int));
				}
				else {
					printf("NULL\n");
				}
			}

			printf("\n");
			break;
	}

END:

	if (NULL != cursor) {
		cursor->destroy(&cursor);
	}

	if (NULL != ion_record.key) {
		free(ion_record.key);
	}

	;

	if (NULL != ion_record.value) {
		free(ion_record.value);
	}

	;

	ion_close_dictionary(&dictionary);

	return error;
}

ion_err_t
create_table(
	iinq_table_id_t		table_id,
	ion_key_type_t		keyType,
	ion_key_size_t		keySize,
	ion_value_size_t	value_size
) {
	ion_err_t error = iinq_create_source(table_id, keyType, keySize, value_size);

	return error;
}

iinq_prepared_sql *
iinq_insert_0(
	int		*value_1,
	char	*value_2,
	int		*value_3
) {
	iinq_prepared_sql *p = malloc(sizeof(iinq_prepared_sql));

	if (NULL == p) {
		return NULL;
	}

	p->table	= 0;
	p->value	= malloc(IINQ_BITS_FOR_NULL(3) + (sizeof(int) * 2) + (sizeof(char) * 31));

	if (NULL == p->value) {
		free(p);
		return NULL;
	}

	iinq_null_indicator_t	*null_indicators	= p->value;
	unsigned char			*data				= ((char *) p->value + IINQ_BITS_FOR_NULL(3));

	p->key = malloc(sizeof(int));

	if (NULL == p->key) {
		free(p->value);
		free(p);
		return NULL;
	}

	if (NULL != value_1) {
		*(int *) ((char *) p->key + 0) = NEUTRALIZE(value_1, int);
	}

	if (NULL == value_1) {
		iinq_set_null_indicator(null_indicators, 1);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 1);
		*(int *) data	= NEUTRALIZE(value_1, int);
		data			+= sizeof(int);
	}

	if (NULL == value_2) {
		iinq_set_null_indicator(null_indicators, 2);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 2);
		strncpy(data, value_2, (sizeof(char) * 31));
		data += (sizeof(char) * 31);
	}

	if (NULL == value_3) {
		iinq_set_null_indicator(null_indicators, 3);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 3);
		*(int *) data = NEUTRALIZE(value_3, int);
	}

	return p;
}

ion_boolean_t
iinq_selection_next(
	iinq_query_operator_t *operator
) {
	int					i;
	ion_boolean_t		selection_result;
	iinq_selection_t	*selection = (iinq_selection_t *) operator->instance;

	do {
		if (!selection->super.input_operators[0]->next(selection->super.input_operators[0])) {
			return boolean_false;
		}

		ion_value_t curr_value;

		selection_result = boolean_true;

		iinq_where_params_t *curr_condition;

		for (i = 0; i < selection->num_conditions; i++) {
			curr_condition = &selection->conditions[i];

			iinq_table_id_t		table_id	= operator->instance->input_operators[0]->instance->field_info[curr_condition->where_field - 1].table_id;
			iinq_field_num_t	field_num	= operator->instance->input_operators[0]->instance->field_info[curr_condition->where_field - 1].field_num;

			curr_value = selection->super.fields[curr_condition->where_field - 1];

			iinq_field_t field_type = iinq_get_field_type(table_id, field_num);

			if (field_type == iinq_int) {
				int comp_value = NEUTRALIZE(curr_condition->field_value, int);

				if (curr_condition->bool_operator == iinq_equal) {
					if (NEUTRALIZE(curr_value, int) != comp_value) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_not_equal) {
					if (NEUTRALIZE(curr_value, int) == comp_value) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_less_than) {
					if (NEUTRALIZE(curr_value, int) >= comp_value) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_greater_than) {
					if (NEUTRALIZE(curr_value, int) <= comp_value) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_less_than_equal_to) {
					if (NEUTRALIZE(curr_value, int) > comp_value) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_greater_than_equal_to) {
					if (NEUTRALIZE(curr_value, int) < comp_value) {
						selection_result = boolean_false;
						break;
					}
				}
			}
			else if (field_type == iinq_null_terminated_string) {
				char	*comp_value = (char *) curr_condition->field_value;
				size_t	value_size	= iinq_calculate_offset(table_id, field_num + 1) - iinq_calculate_offset(table_id, field_num);

				if (curr_condition->bool_operator == iinq_equal) {
					if (strncmp((char *) curr_value, comp_value, value_size) != 0) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_not_equal) {
					if (strncmp((char *) curr_value, comp_value, value_size) == 0) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_less_than) {
					if (strncmp((char *) curr_value, comp_value, value_size) >= 0) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_greater_than) {
					if (strncmp((char *) curr_value, comp_value, value_size) <= 0) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_less_than_equal_to) {
					if (strncmp((char *) curr_value, comp_value, value_size) > 0) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_greater_than_equal_to) {
					if (strncmp((char *) curr_value, comp_value, value_size) < 0) {
						selection_result = boolean_false;
						break;
					}
				}
			}
			else if (field_type == iinq_char_array) {
				unsigned char	*comp_value = (char *) curr_condition->field_value;
				size_t			value_size	= iinq_calculate_offset(table_id, field_num + 1) - iinq_calculate_offset(table_id, field_num);

				if (curr_condition->bool_operator == iinq_equal) {
					if (memcmp((char *) curr_value, comp_value, value_size) != 0) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_not_equal) {
					if (memcmp((char *) curr_value, comp_value, value_size) == 0) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_less_than) {
					if (memcmp((char *) curr_value, comp_value, value_size) >= 0) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_greater_than) {
					if (memcmp((char *) curr_value, comp_value, value_size) <= 0) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_less_than_equal_to) {
					if (memcmp((char *) curr_value, comp_value, value_size) > 0) {
						selection_result = boolean_false;
						break;
					}
				}
				else if (curr_condition->bool_operator == iinq_greater_than_equal_to) {
					if (memcmp((char *) curr_value, comp_value, value_size) < 0) {
						selection_result = boolean_false;
						break;
					}
				}
			}
		}
	} while (!selection_result);

	if (selection_result) {
		operator->status.count++;
	}

	return selection_result;
}
