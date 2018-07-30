#include "iinq_user_functions.h"

iinq_query_operator_t *
iinq_selection_init(
	iinq_query_operator_t	*input_operator,
	unsigned int			num_conditions,
	iinq_where_params_t		*conditions
) {
	iinq_query_operator_t *operator = malloc(sizeof(iinq_query_operator_t));

	if (NULL == operator) {
		goto ERROR;
	}

	operator->instance = malloc(sizeof(iinq_selection_t));

	if (NULL == operator->instance) {
		goto ERROR;
	}

	iinq_selection_t *selection = (iinq_selection_t *) operator->instance;

	if (NULL == input_operator) {
		goto ERROR;
	}

	selection->super.type					= iinq_selection_e;
	selection->super.num_input_operators	= 1;
	selection->conditions					= NULL;
	selection->super.input_operators		= malloc(sizeof(iinq_query_operator_t *));

	if (NULL == selection->super.input_operators) {
		goto ERROR;
	}

	selection->super.input_operators[0] = input_operator;
	selection->super.num_fields			= input_operator->instance->num_fields;
	selection->super.null_indicators	= input_operator->instance->null_indicators;
	selection->super.field_info			= input_operator->instance->field_info;
	selection->super.fields				= input_operator->instance->fields;

	selection->num_conditions			= num_conditions;
	selection->conditions				= malloc(sizeof(iinq_where_params_t) * num_conditions);

	if (NULL == selection->conditions) {
		goto ERROR;
	}

	int i;

	for (i = 0; i < num_conditions; i++) {
		selection->conditions[i].where_field			= conditions[i].where_field;
		selection->conditions[i].field_info.field_num	= conditions[i].field_info.field_num;

		size_t value_size = iinq_calculate_offset(input_operator->instance->field_info[i].table_id, input_operator->instance->field_info[i].field_num + 1) - iinq_calculate_offset(input_operator->instance->field_info[i].table_id, input_operator->instance->field_info[i].field_num);

		selection->conditions[i].field_value = malloc(value_size);

		if (NULL == selection->conditions[i].field_value) {
			goto ERROR;
		}

		selection->conditions[i].bool_operator = conditions[i].bool_operator;

		switch (iinq_get_field_type(input_operator->instance->field_info[i].table_id, input_operator->instance->field_info[i].field_num)) {
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

ERROR:;
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

		case 1:

			switch (field_num) {
				case 1:
					return 0;
			}

		case 2:

			switch (field_num) {
				case 1:
					return 0;
			}

		case 3:

			switch (field_num) {
				case 1:
					return 0;

				case 2:
					return 0 + sizeof(int);
			}

		case 4:

			switch (field_num) {
				case 2:
					return 0;

				case 1:
					return 0 + sizeof(int);
			}
	}
}

iinq_query_operator_t *
iinq_projection_init(
	iinq_query_operator_t	*input_operator,
	iinq_field_num_t		num_fields,
	iinq_field_num_t		*field_nums
) {
	iinq_projection_t		*projection = NULL;
	iinq_query_operator_t	*operator	= malloc(sizeof(iinq_query_operator_t));

	if (NULL == operator) {
		goto ERROR;
	}

	operator->instance = malloc(sizeof(iinq_projection_t));

	if (NULL == operator->instance) {
		goto ERROR;
	}

	operator->next			= iinq_projection_next;
	operator->destroy		= iinq_projection_destroy;

	projection				= operator->instance;
	projection->super.type	= iinq_projection_e;

	if (input_operator == NULL) {
		goto ERROR;
	}

	projection->super.num_input_operators	= 1;
	projection->super.input_operators		= malloc(sizeof(iinq_query_operator_t));

	if (NULL == projection->super.input_operators) {
		goto ERROR;
	}

	projection->super.input_operators[0]	= input_operator;
	projection->super.num_fields			= num_fields;

	projection->input_field_nums			= malloc(sizeof(iinq_field_num_t) * num_fields);

	if (NULL == projection->input_field_nums) {
		goto ERROR;
	}

	memcpy(projection->input_field_nums, field_nums, sizeof(iinq_field_num_t) * num_fields);

	projection->super.null_indicators = malloc(IINQ_BITS_FOR_NULL(num_fields));

	if (NULL == projection->super.null_indicators) {
		goto ERROR;
	}

	projection->super.field_info = malloc(sizeof(iinq_field_info_t) * num_fields);

	if (NULL == projection->super.field_info) {
		goto ERROR;
	}

	projection->super.fields = malloc(sizeof(ion_value_t) * num_fields);

	if (NULL == projection->super.fields) {
		goto ERROR;
	}

	int i;

	for (i = 0; i < num_fields; i++) {
		projection->super.field_info[i] = input_operator->instance->field_info[field_nums[i] - 1];
		projection->super.fields[i]		= input_operator->instance->fields[field_nums[i] - 1];
	}

	operator->status = ION_STATUS_OK(0);

	return operator;

ERROR:

	if (NULL != input_operator) {
		input_operator->destroy(&input_operator);
	}

	if (NULL != operator) {
		if (NULL != projection) {
			if (NULL != projection->super.input_operators) {
				free(projection->super.input_operators);
			}

			free(projection);
		}

		free(operator);
	}

	return NULL;
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

void
update(
	iinq_table_id_t table_id,
	int				num_wheres,
	int				num_update,
	...
) {
	va_list valist;

	va_start(valist, num_update);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
	}

	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(&dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(dictionary.instance->record.key_size);
	ion_record.value	= malloc(dictionary.instance->record.value_size);

	ion_key_t			new_key = malloc(dictionary.instance->record.key_size);
	ion_cursor_status_t status;

	ion_dictionary_handler_t	handler_temp;
	ion_dictionary_t			dictionary_temp;

	iinq_update_handler_init(&handler_temp);
	dictionary_temp.handler = &handler_temp;

	error					= ion_init_master_table();
	error					= ion_master_table_create_dictionary(&handler_temp, &dictionary_temp, dictionary.instance->key_type, dictionary.instance->record.key_size, dictionary.instance->record.value_size, 10);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
	}

	ion_boolean_t condition_satisfied;

	iinq_where_params_t *wheres = NULL;

	if (num_wheres > 0) {
		wheres = va_arg(valist, iinq_where_params_t *);
	}

	while ((status = cursor->next(cursor, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		condition_satisfied = where(table_id, &ion_record, num_wheres, wheres);

		if (condition_satisfied) {
			error = dictionary_insert(&dictionary_temp, ion_record.key, ion_record.value).error;

			if (err_ok != error) {
				printf("Error occurred. Error code: %i\n", error);
			}
		}
	}

	cursor->destroy(&cursor);

	int i;

	iinq_update_params_t *updates = va_arg(valist, iinq_update_params_t *);

	va_end(valist);

	ion_predicate_t predicate_temp;

	dictionary_build_predicate(&predicate_temp, predicate_all_records);

	ion_dict_cursor_t *cursor_temp = NULL;

	dictionary_find(&dictionary_temp, &predicate_temp, &cursor_temp);

	while ((status = cursor_temp->next(cursor_temp, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		ion_boolean_t key_changed = boolean_false;

		memcpy(new_key, ion_record.key, dictionary.instance->record.key_size);

		for (i = 0; i < num_update; i++) {
			unsigned char *value;

			if (updates[i].implicit_field != 0) {
				int new_value;

				value = (char *) ion_record.value + iinq_calculate_offset(table_id, updates[i].implicit_field);

				switch (updates[i].math_operator) {
					case iinq_add:
						new_value = (NEUTRALIZE(value, int) + NEUTRALIZE(updates[i].field_value, int));
						break;

					case iinq_subtract:
						new_value = (NEUTRALIZE(value, int) - NEUTRALIZE(updates[i].field_value, int));
						break;

					case iinq_multiply:
						new_value = (NEUTRALIZE(value, int) * NEUTRALIZE(updates[i].field_value, int));
						break;

					case iinq_divide:
						new_value = (NEUTRALIZE(value, int) / NEUTRALIZE(updates[i].field_value, int));
						break;
				}

				if (iinq_is_key_field(table_id, updates[i].update_field)) {
					*(int *) ((char *) new_key + iinq_calculate_key_offset(table_id, updates[i].update_field))	= new_value;
					key_changed																					= boolean_true;
				}

				value			= (char *) ion_record.value + iinq_calculate_offset(table_id, updates[i].update_field);
				*(int *) value	= new_value;
			}
			else {
				value = (char *) ion_record.value + iinq_calculate_offset(table_id, updates[i].update_field);

				if (iinq_get_field_type(table_id, updates[i].update_field) == iinq_int) {
					if (iinq_is_key_field(table_id, updates[i].update_field)) {
						*(int *) ((char *) new_key + iinq_calculate_key_offset(table_id, updates[i].update_field))	= NEUTRALIZE(updates[i].field_value, int);
						key_changed																					= boolean_true;
					}

					*(int *) value = NEUTRALIZE(updates[i].field_value, int);
				}
				else {
					size_t size = iinq_calculate_offset(table_id, updates[i].update_field + 1) - iinq_calculate_offset(table_id, updates[i].update_field);

					if (iinq_is_key_field(table_id, updates[i].update_field)) {
						strncpy((char *) new_key + iinq_calculate_key_offset(table_id, updates[i].update_field), updates[i].field_value, size);
						key_changed = boolean_true;
					}

					strncpy(value, updates[i].field_value, size);
				}
			}
		}

		if (key_changed) {
			ion_predicate_t		dup_predicate;
			ion_dict_cursor_t	*dup_cursor = NULL;

			dictionary_build_predicate(&dup_predicate, predicate_equality, new_key);
			dictionary_find(&dictionary, &dup_predicate, &dup_cursor);

			if (cs_end_of_results != dup_cursor->status) {
				error = err_duplicate_key;
			}
			else {
				ion_status_t ion_status;

				ion_status	= dictionary_delete(&dictionary, ion_record.key);
				ion_status	= dictionary_insert(&dictionary, new_key, ion_record.value);
				error		= err_ok;
			}

			dup_cursor->destroy(&dup_cursor);
		}
		else {
			error = dictionary_update(&dictionary, ion_record.key, ion_record.value).error;
		}

		if (err_ok != error) {
			printf("Error occurred. Error code: %i\n", error);
		}
	}

	cursor_temp->destroy(&cursor_temp);
	ion_delete_dictionary(&dictionary_temp, dictionary_temp.instance->id);
	ion_close_dictionary(&dictionary);
	ion_close_master_table();
	free(new_key);
	free(ion_record.key);
	free(ion_record.value);
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
	iinq_table_scan_t			*table_scan = NULL;
	ion_predicate_t				*predicate	= NULL;
	ion_dict_cursor_t			*cursor		= NULL;
	ion_record_t				*record		= NULL;
	ion_dictionary_t			*dictionary = NULL;
	ion_dictionary_handler_t	*handler	= NULL;

	iinq_query_operator_t *operator			= malloc(sizeof(iinq_query_operator_t));

	if (NULL == operator) {
		goto ERROR;
	}

	operator->instance = malloc(sizeof(iinq_table_scan_t));

	if (NULL == operator->instance) {
		goto ERROR;
	}

	table_scan				= (iinq_table_scan_t *) operator->instance;
	table_scan->super.type	= iinq_table_scan_e;
	predicate				= &table_scan->predicate;
	error					= dictionary_build_predicate(predicate, predicate_all_records);

	if (err_ok != error) {
		goto ERROR;
	}

	dictionary			= &table_scan->dictionary;
	handler				= &table_scan->handler;
	dictionary->handler = handler;
	record				= &table_scan->record;
	record->key			= NULL;
	record->value		= NULL;

	error				= iinq_open_source(table_id, dictionary, handler);

	if (err_ok != error) {
		goto ERROR;
	}

	error = dictionary_find(dictionary, predicate, &cursor);

	if (err_ok != error) {
		goto ERROR;
	}

	table_scan->cursor		= cursor;
	table_scan->record.key	= malloc(dictionary->instance->record.key_size);

	if (NULL == table_scan->record.key) {
		goto ERROR;
	}

	record->value = malloc(dictionary->instance->record.value_size);

	if (NULL == record->value) {
		goto ERROR;
	}

	table_scan->super.field_info = malloc(sizeof(iinq_field_info_t) * num_fields);

	if (NULL == table_scan->super.field_info) {
		goto ERROR;
	}

	table_scan->super.null_indicators	= table_scan->record.value;

	table_scan->super.fields			= malloc(sizeof(ion_value_t) * num_fields);

	if (NULL == table_scan->super.fields) {
		goto ERROR;
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

ERROR:

	if (NULL != record) {
		if (NULL != record->key) {
			free(record->key);
		}

		if (NULL != record->value) {
			free(record->value);
		}
	}

	if (NULL != cursor) {
		cursor->destroy(&cursor);
	}

	if (NULL != dictionary) {
		ion_close_dictionary(dictionary);
	}

	if (NULL != operator) {
		if (NULL != operator->instance) {
			free(operator->instance);
		}

		free(operator);
	}

	return NULL;
}

void
delete_record(
	iinq_table_id_t table_id,
	int				num_wheres,
	...
) {
	va_list valist;

	va_start(valist, num_wheres);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(&dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(dictionary.instance->record.key_size);
	ion_record.value	= malloc(dictionary.instance->record.value_size);

	ion_cursor_status_t status;

	ion_dictionary_t			dictionary_temp;
	ion_dictionary_handler_t	handler_temp;

	dictionary_temp.handler = &handler_temp;

	error					= ion_init_master_table();

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	iinq_delete_handler_init(&handler_temp);
	dictionary_temp.handler = &handler_temp;

	error					= ion_master_table_create_dictionary(&handler_temp, &dictionary_temp, dictionary.instance->key_type, dictionary.instance->record.key_size, 1, 10);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	ion_boolean_t condition_satisfied;

	iinq_where_params_t *wheres = NULL;

	if (num_wheres > 0) {
		wheres = va_arg(valist, iinq_where_params_t *);
	}

	while ((status = cursor->next(cursor, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		condition_satisfied = where(table_id, &ion_record, num_wheres, wheres);

		if (condition_satisfied) {
			error = dictionary_insert(&dictionary_temp, ion_record.key, IONIZE(0, char)).error;

			if (err_ok != error) {
				printf("Error occurred. Error code: %i\n", error);
				return;
			}
		}
	}

	va_end(valist);
	cursor->destroy(&cursor);

	ion_predicate_t predicate_temp;

	dictionary_build_predicate(&predicate_temp, predicate_all_records);

	ion_dict_cursor_t *cursor_temp = NULL;

	dictionary_find(&dictionary_temp, &predicate_temp, &cursor_temp);

	while ((status = cursor_temp->next(cursor_temp, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		error = dictionary_delete(&dictionary, ion_record.key).error;

		if (err_ok != error) {
			printf("Error occurred. Error code: %i\n", error);
			return;
		}
	}

	cursor_temp->destroy(&cursor_temp);
	error = ion_delete_dictionary(&dictionary_temp, dictionary_temp.instance->id);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	ion_close_dictionary(&dictionary);
	ion_close_master_table();
	free(ion_record.key);
	free(ion_record.value);
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

void
iinq_selection_destroy(
	iinq_query_operator_t **operator
) {
	if (NULL != *operator) {
		if (NULL != (*operator)->instance) {
			iinq_selection_t *selection = (iinq_selection_t *) (*operator)->instance;

			if (NULL != selection->super.input_operators) {
				if (NULL != selection->super.input_operators[0]) {
					selection->super.input_operators[0]->destroy(&selection->super.input_operators[0]);
				}

				free(selection->super.input_operators);
			}

			if (NULL != selection->conditions) {
				int i;

				for (i = 0; i < selection->num_conditions; i++) {
					free(selection->conditions[i].field_value);
				}

				free(selection->conditions);
			}

			free(selection);
		}

		free(*operator);
		*operator = NULL;
	}
}

void
iinq_table_scan_destroy(
	iinq_query_operator_t **operator
) {
	iinq_table_scan_t *table_scan = (iinq_table_scan_t *) (*operator)->instance;

	if (NULL != table_scan->record.key) {
		free(table_scan->record.key);
	}

	if (NULL != table_scan->record.value) {
		free(table_scan->record.value);
	}

	ion_close_dictionary(&table_scan->dictionary);

	if (NULL != table_scan->cursor) {
		table_scan->cursor->destroy(&table_scan->cursor);
	}

	free(table_scan);
	free(*operator);
	*operator = NULL;
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

			ion_record.value = malloc(IINQ_BITS_FOR_NULL(5) + (sizeof(int) * 2) + (sizeof(char) * 83));

			if (NULL == ion_record.value) {
				error = err_out_of_memory;
				goto END;
			}

			break;

		case 1:
			ion_record.key = malloc((sizeof(char) * 3));

			if (NULL == ion_record.key) {
				error = err_out_of_memory;
				goto END;
			}

			ion_record.value = malloc(IINQ_BITS_FOR_NULL(5) + (sizeof(int) * 1) + (sizeof(char) * 86));

			if (NULL == ion_record.value) {
				error = err_out_of_memory;
				goto END;
			}

			break;

		case 2:
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

		case 3:
			ion_record.key = malloc(sizeof(int) + sizeof(int));

			if (NULL == ion_record.key) {
				error = err_out_of_memory;
				goto END;
			}

			ion_record.value = malloc(IINQ_BITS_FOR_NULL(3) + (sizeof(int) * 2) + (sizeof(char) * 6));

			if (NULL == ion_record.value) {
				error = err_out_of_memory;
				goto END;
			}

			break;

		case 4:
			ion_record.key = malloc(sizeof(int) + sizeof(int));

			if (NULL == ion_record.key) {
				error = err_out_of_memory;
				goto END;
			}

			ion_record.value = malloc(IINQ_BITS_FOR_NULL(3) + (sizeof(int) * 2) + (sizeof(char) * 6));

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
				value = ((unsigned char *) ion_record.value) + IINQ_BITS_FOR_NULL(5);

				if (!iinq_check_null_indicator(null_indicators, 1)) {
					printf("%10d, ", NEUTRALIZE(value, int));
				}
				else {
					printf("NULL, ");
				}

				value += sizeof(int);

				if (!iinq_check_null_indicator(null_indicators, 2)) {
					printf("%21s, ", (char *) value);
				}
				else {
					printf("NULL, ");
				}

				value += (sizeof(char) * 21);

				if (!iinq_check_null_indicator(null_indicators, 3)) {
					printf("%31s, ", (char *) value);
				}
				else {
					printf("NULL, ");
				}

				value += (sizeof(char) * 31);

				if (!iinq_check_null_indicator(null_indicators, 4)) {
					printf("%10d, ", NEUTRALIZE(value, int));
				}
				else {
					printf("NULL, ");
				}

				value += sizeof(int);

				if (!iinq_check_null_indicator(null_indicators, 5)) {
					printf("%31s\n", (char *) value);
				}
				else {
					printf("NULL\n");
				}
			}

			printf("\n");
			break;

		case 1:
			null_indicators = ion_record.value;

			while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				value = ((unsigned char *) ion_record.value) + IINQ_BITS_FOR_NULL(5);

				if (!iinq_check_null_indicator(null_indicators, 1)) {
					printf("%3s, ", (char *) value);
				}
				else {
					printf("NULL, ");
				}

				value += (sizeof(char) * 3);

				if (!iinq_check_null_indicator(null_indicators, 2)) {
					printf("%21s, ", (char *) value);
				}
				else {
					printf("NULL, ");
				}

				value += (sizeof(char) * 21);

				if (!iinq_check_null_indicator(null_indicators, 3)) {
					printf("%31s, ", (char *) value);
				}
				else {
					printf("NULL, ");
				}

				value += (sizeof(char) * 31);

				if (!iinq_check_null_indicator(null_indicators, 4)) {
					printf("%10d, ", NEUTRALIZE(value, int));
				}
				else {
					printf("NULL, ");
				}

				value += sizeof(int);

				if (!iinq_check_null_indicator(null_indicators, 5)) {
					printf("%31s\n", (char *) value);
				}
				else {
					printf("NULL\n");
				}
			}

			printf("\n");
			break;

		case 2:
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

		case 3:
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
					printf("%10d, ", NEUTRALIZE(value, int));
				}
				else {
					printf("NULL, ");
				}

				value += sizeof(int);

				if (!iinq_check_null_indicator(null_indicators, 3)) {
					printf("%6s\n", (char *) value);
				}
				else {
					printf("NULL\n");
				}
			}

			printf("\n");
			break;

		case 4:
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
					printf("%10d, ", NEUTRALIZE(value, int));
				}
				else {
					printf("NULL, ");
				}

				value += sizeof(int);

				if (!iinq_check_null_indicator(null_indicators, 3)) {
					printf("%6s\n", (char *) value);
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

ion_boolean_t
iinq_selection_next(
	iinq_query_operator_t *operator
) {
	int					i;
	iinq_selection_t	*selection			= (iinq_selection_t *) operator->instance;
	ion_boolean_t		selection_result	= boolean_false;

	while (!selection_result && selection->super.input_operators[0]->next(selection->super.input_operators[0])) {
		ion_value_t			curr_value;
		iinq_where_params_t *curr_condition;

		for (i = 0; i < selection->num_conditions; i++) {
			curr_value		= selection->super.fields[selection->conditions[i].where_field - 1];
			curr_condition	= &selection->conditions[i];

			iinq_field_t field_type = iinq_get_field_type(curr_condition->field_info.table_id, curr_condition->field_info.field_num);

			switch (field_type) {
				case iinq_int: {
					int comp_value = NEUTRALIZE(curr_condition->field_value, int);

					switch (curr_condition->bool_operator) {
						case iinq_equal:

							if (NEUTRALIZE(curr_value, int) != comp_value) {
								selection_result = boolean_false;
							}

							break;

						case iinq_not_equal:

							if (NEUTRALIZE(curr_value, int) == comp_value) {
								selection_result = boolean_false;
							}

							break;

						case iinq_less_than:

							if (NEUTRALIZE(curr_value, int) >= comp_value) {
								selection_result = boolean_false;
							}

							break;

						case iinq_less_than_equal_to:

							if (NEUTRALIZE(curr_value, int) > comp_value) {
								selection_result = boolean_false;
							}

							break;

						case iinq_greater_than_equal_to:

							if (NEUTRALIZE(curr_value, int) < comp_value) {
								selection_result = boolean_false;
							}

							break;
					}

					break;
				}

				case iinq_null_terminated_string: {
					char	*comp_value = (char *) curr_condition->field_value;
					size_t	value_size	= iinq_calculate_offset(curr_condition->field_info.table_id, curr_condition->field_info.field_num + 1) - iinq_calculate_offset(curr_condition->field_info.table_id, curr_condition->field_info.field_num);

					switch (curr_condition->bool_operator) {
						case iinq_equal:

							if (strncmp((char *) curr_value, comp_value, value_size) != 0) {
								selection_result = boolean_false;
							}

							break;

						case iinq_not_equal:

							if (strncmp((char *) curr_value, comp_value, value_size) == 0) {
								selection_result = boolean_false;
							}

							break;

						case iinq_less_than:

							if (strncmp((char *) curr_value, comp_value, value_size) >= 0) {
								selection_result = boolean_false;
							}

							break;

						case iinq_less_than_equal_to:

							if (strncmp((char *) curr_value, comp_value, value_size) > 0) {
								selection_result = boolean_false;
							}

							break;

						case iinq_greater_than_equal_to:

							if (strncmp((char *) curr_value, comp_value, value_size) < 0) {
								selection_result = boolean_false;
							}

							break;
					}

					break;
				}

				case iinq_char_array: {
					unsigned char	*comp_value = (unsigned char *) curr_condition->field_value;
					size_t			value_size	= iinq_calculate_offset(curr_condition->field_info.table_id, curr_condition->field_info.field_num + 1) - iinq_calculate_offset(curr_condition->field_info.table_id, curr_condition->field_info.field_num);

					switch (curr_condition->bool_operator) {
						case iinq_equal:

							if (memcmp((char *) curr_value, comp_value, value_size) != 0) {
								selection_result = boolean_false;
							}

							break;

						case iinq_not_equal:

							if (memcmp((char *) curr_value, comp_value, value_size) == 0) {
								selection_result = boolean_false;
							}

							break;

						case iinq_less_than:

							if (memcmp((char *) curr_value, comp_value, value_size) >= 0) {
								selection_result = boolean_false;
							}

							break;

						case iinq_less_than_equal_to:

							if (memcmp((char *) curr_value, comp_value, value_size) > 0) {
								selection_result = boolean_false;
							}

							break;

						case iinq_greater_than_equal_to:

							if (memcmp((char *) curr_value, comp_value, value_size) < 0) {
								selection_result = boolean_false;
							}

							break;
					}

					break;
				}
			}
		}
	}

	return selection_result;
}

size_t
iinq_calculate_offset(
	iinq_table_id_t		table_id,
	iinq_field_num_t	field_num
) {
	switch (table_id) {
		case 0: {
			switch (field_num) {
				case 1:
					return IINQ_BITS_FOR_NULL(5);

				case 2:
					return IINQ_BITS_FOR_NULL(5) + sizeof(int);

				case 3:
					return IINQ_BITS_FOR_NULL(5) + sizeof(int) + (sizeof(char) * 21);

				case 4:
					return IINQ_BITS_FOR_NULL(5) + sizeof(int) + (sizeof(char) * 21) + (sizeof(char) * 31);

				case 5:
					return IINQ_BITS_FOR_NULL(5) + sizeof(int) + (sizeof(char) * 21) + (sizeof(char) * 31) + sizeof(int);

				case 6:
					return IINQ_BITS_FOR_NULL(5) + sizeof(int) + (sizeof(char) * 21) + (sizeof(char) * 31) + sizeof(int) + (sizeof(char) * 31);

				default:
					return 0;
			}
		}

		case 1: {
			switch (field_num) {
				case 1:
					return IINQ_BITS_FOR_NULL(5);

				case 2:
					return IINQ_BITS_FOR_NULL(5) + (sizeof(char) * 3);

				case 3:
					return IINQ_BITS_FOR_NULL(5) + (sizeof(char) * 3) + (sizeof(char) * 21);

				case 4:
					return IINQ_BITS_FOR_NULL(5) + (sizeof(char) * 3) + (sizeof(char) * 21) + (sizeof(char) * 31);

				case 5:
					return IINQ_BITS_FOR_NULL(5) + (sizeof(char) * 3) + (sizeof(char) * 21) + (sizeof(char) * 31) + sizeof(int);

				case 6:
					return IINQ_BITS_FOR_NULL(5) + (sizeof(char) * 3) + (sizeof(char) * 21) + (sizeof(char) * 31) + sizeof(int) + (sizeof(char) * 31);

				default:
					return 0;
			}
		}

		case 2: {
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

		case 3: {
			switch (field_num) {
				case 1:
					return IINQ_BITS_FOR_NULL(3);

				case 2:
					return IINQ_BITS_FOR_NULL(3) + sizeof(int);

				case 3:
					return IINQ_BITS_FOR_NULL(3) + sizeof(int) + sizeof(int);

				case 4:
					return IINQ_BITS_FOR_NULL(3) + sizeof(int) + sizeof(int) + (sizeof(char) * 6);

				default:
					return 0;
			}
		}

		case 4: {
			switch (field_num) {
				case 1:
					return IINQ_BITS_FOR_NULL(3);

				case 2:
					return IINQ_BITS_FOR_NULL(3) + sizeof(int);

				case 3:
					return IINQ_BITS_FOR_NULL(3) + sizeof(int) + sizeof(int);

				case 4:
					return IINQ_BITS_FOR_NULL(3) + sizeof(int) + sizeof(int) + (sizeof(char) * 6);

				default:
					return 0;
			}
		}

		default:
			return 0;
	}
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

		case 1: {
			if (iinq_check_null_indicator(p->value, 1)) {
				return err_unable_to_insert;
			}

			return iinq_execute(1, p->key, p->value, iinq_insert_t);
		}

		case 2: {
			if (iinq_check_null_indicator(p->value, 1)) {
				return err_unable_to_insert;
			}

			return iinq_execute(2, p->key, p->value, iinq_insert_t);
		}

		case 3: {
			if (iinq_check_null_indicator(p->value, 1) || iinq_check_null_indicator(p->value, 2)) {
				return err_unable_to_insert;
			}

			return iinq_execute(3, p->key, p->value, iinq_insert_t);
		}

		case 4: {
			if (iinq_check_null_indicator(p->value, 2) || iinq_check_null_indicator(p->value, 1)) {
				return err_unable_to_insert;
			}

			return iinq_execute(4, p->key, p->value, iinq_insert_t);
		}
	}
}

void
iinq_projection_destroy(
	iinq_query_operator_t **operator
) {
	if (NULL != *operator) {
		iinq_projection_t *projection = (iinq_projection_t *) (*operator)->instance;

		if (NULL != projection->super.input_operators) {
			projection->super.input_operators[0]->destroy(projection->super.input_operators);
			free(projection->super.input_operators);
		}

		*operator = NULL;
	}
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

			ion_record.value = malloc(IINQ_BITS_FOR_NULL(5) + (sizeof(int) * 2) + (sizeof(char) * 83));

			if (NULL == ion_record.value) {
				error = err_out_of_memory;
				goto END;
			}

			while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				key = ion_record.key;

				printf("%10d\n", NEUTRALIZE(key, int));
			}

			break;

		case 1:
			ion_record.key = malloc((sizeof(char) * 3));

			if (NULL == ion_record.key) {
				error = err_out_of_memory;
				goto END;
			}

			ion_record.value = malloc(IINQ_BITS_FOR_NULL(5) + (sizeof(int) * 1) + (sizeof(char) * 86));

			if (NULL == ion_record.value) {
				error = err_out_of_memory;
				goto END;
			}

			while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				key = ion_record.key;

				printf("%3s\n", (char *) key);
			}

			break;

		case 2:
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

		case 3:
			ion_record.key = malloc(sizeof(int) + sizeof(int));

			if (NULL == ion_record.key) {
				error = err_out_of_memory;
				goto END;
			}

			ion_record.value = malloc(IINQ_BITS_FOR_NULL(3) + (sizeof(int) * 2) + (sizeof(char) * 6));

			if (NULL == ion_record.value) {
				error = err_out_of_memory;
				goto END;
			}

			while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				key = ion_record.key;

				printf("%10d, ", NEUTRALIZE(key, int));
				key += sizeof(int);

				printf("%10d\n", NEUTRALIZE(key, int));
			}

			break;

		case 4:
			ion_record.key = malloc(sizeof(int) + sizeof(int));

			if (NULL == ion_record.key) {
				error = err_out_of_memory;
				goto END;
			}

			ion_record.value = malloc(IINQ_BITS_FOR_NULL(3) + (sizeof(int) * 2) + (sizeof(char) * 6));

			if (NULL == ion_record.value) {
				error = err_out_of_memory;
				goto END;
			}

			while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				key = ion_record.key;

				printf("%10d, ", NEUTRALIZE(key, int));
				key += sizeof(int);

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

iinq_prepared_sql *
iinq_insert_4(
	int		*value_1,
	int		*value_2,
	char	*value_3
) {
	iinq_prepared_sql *p = malloc(sizeof(iinq_prepared_sql));

	if (NULL == p) {
		return NULL;
	}

	p->table	= 4;
	p->value	= malloc(IINQ_BITS_FOR_NULL(3) + (sizeof(int) * 2) + (sizeof(char) * 6));

	if (NULL == p->value) {
		free(p);
		return NULL;
	}

	iinq_null_indicator_t	*null_indicators	= p->value;
	unsigned char			*data				= ((char *) p->value + IINQ_BITS_FOR_NULL(3));

	p->key = malloc(sizeof(int) + sizeof(int));

	if (NULL == p->key) {
		free(p->value);
		free(p);
		return NULL;
	}

	if (NULL != value_2) {
		*(int *) ((char *) p->key + 0) = NEUTRALIZE(value_2, int);
	}

	if (NULL != value_1) {
		*(int *) ((char *) p->key + 0 + sizeof(int)) = NEUTRALIZE(value_1, int);
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
		*(int *) data	= NEUTRALIZE(value_2, int);
		data			+= sizeof(int);
	}

	if (NULL == value_3) {
		iinq_set_null_indicator(null_indicators, 3);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 3);
		strncpy(data, value_3, (sizeof(char) * 6));
	}

	return p;
}

iinq_prepared_sql *
iinq_insert_2(
	int		*value_1,
	char	*value_2,
	int		*value_3
) {
	iinq_prepared_sql *p = malloc(sizeof(iinq_prepared_sql));

	if (NULL == p) {
		return NULL;
	}

	p->table	= 2;
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

iinq_prepared_sql *
iinq_insert_3(
	int		*value_1,
	int		*value_2,
	char	*value_3
) {
	iinq_prepared_sql *p = malloc(sizeof(iinq_prepared_sql));

	if (NULL == p) {
		return NULL;
	}

	p->table	= 3;
	p->value	= malloc(IINQ_BITS_FOR_NULL(3) + (sizeof(int) * 2) + (sizeof(char) * 6));

	if (NULL == p->value) {
		free(p);
		return NULL;
	}

	iinq_null_indicator_t	*null_indicators	= p->value;
	unsigned char			*data				= ((char *) p->value + IINQ_BITS_FOR_NULL(3));

	p->key = malloc(sizeof(int) + sizeof(int));

	if (NULL == p->key) {
		free(p->value);
		free(p);
		return NULL;
	}

	if (NULL != value_1) {
		*(int *) ((char *) p->key + 0) = NEUTRALIZE(value_1, int);
	}

	if (NULL != value_2) {
		*(int *) ((char *) p->key + 0 + sizeof(int)) = NEUTRALIZE(value_2, int);
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
		*(int *) data	= NEUTRALIZE(value_2, int);
		data			+= sizeof(int);
	}

	if (NULL == value_3) {
		iinq_set_null_indicator(null_indicators, 3);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 3);
		strncpy(data, value_3, (sizeof(char) * 6));
	}

	return p;
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

		case 1:

			switch (field_num) {
				case 1:
					return boolean_true;

				default:
					return boolean_false;
			}

		case 2:

			switch (field_num) {
				case 1:
					return boolean_true;

				default:
					return boolean_false;
			}

		case 3:

			switch (field_num) {
				case 1:
				case 2:
					return boolean_true;

				default:
					return boolean_false;
			}

		case 4:

			switch (field_num) {
				case 2:
				case 1:
					return boolean_true;

				default:
					return boolean_false;
			}

		default:
			return boolean_false;
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
					return iinq_null_terminated_string;

				case 4:
					return iinq_int;

				case 5:
					return iinq_null_terminated_string;

				default:
					return 0;
			}
		}

		case 1: {
			switch (field_num) {
				case 1:
					return iinq_null_terminated_string;

				case 2:
					return iinq_null_terminated_string;

				case 3:
					return iinq_null_terminated_string;

				case 4:
					return iinq_int;

				case 5:
					return iinq_null_terminated_string;

				default:
					return 0;
			}
		}

		case 2: {
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

		case 3: {
			switch (field_num) {
				case 1:
					return iinq_int;

				case 2:
					return iinq_int;

				case 3:
					return iinq_null_terminated_string;

				default:
					return 0;
			}
		}

		case 4: {
			switch (field_num) {
				case 1:
					return iinq_int;

				case 2:
					return iinq_int;

				case 3:
					return iinq_null_terminated_string;

				default:
					return 0;
			}
		}

		default:
			return 0;
	}
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
	char	*value_3,
	int		*value_4,
	char	*value_5
) {
	iinq_prepared_sql *p = malloc(sizeof(iinq_prepared_sql));

	if (NULL == p) {
		return NULL;
	}

	p->table	= 0;
	p->value	= malloc(IINQ_BITS_FOR_NULL(5) + (sizeof(int) * 2) + (sizeof(char) * 83));

	if (NULL == p->value) {
		free(p);
		return NULL;
	}

	iinq_null_indicator_t	*null_indicators	= p->value;
	unsigned char			*data				= ((char *) p->value + IINQ_BITS_FOR_NULL(5));

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
		strncpy(data, value_2, (sizeof(char) * 21));
		data += (sizeof(char) * 21);
	}

	if (NULL == value_3) {
		iinq_set_null_indicator(null_indicators, 3);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 3);
		strncpy(data, value_3, (sizeof(char) * 31));
		data += (sizeof(char) * 31);
	}

	if (NULL == value_4) {
		iinq_set_null_indicator(null_indicators, 4);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 4);
		*(int *) data	= NEUTRALIZE(value_4, int);
		data			+= sizeof(int);
	}

	if (NULL == value_5) {
		iinq_set_null_indicator(null_indicators, 5);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 5);
		strncpy(data, value_5, (sizeof(char) * 31));
	}

	return p;
}

iinq_prepared_sql *
iinq_insert_1(
	char	*value_1,
	char	*value_2,
	char	*value_3,
	int		*value_4,
	char	*value_5
) {
	iinq_prepared_sql *p = malloc(sizeof(iinq_prepared_sql));

	if (NULL == p) {
		return NULL;
	}

	p->table	= 1;
	p->value	= malloc(IINQ_BITS_FOR_NULL(5) + (sizeof(int) * 1) + (sizeof(char) * 86));

	if (NULL == p->value) {
		free(p);
		return NULL;
	}

	iinq_null_indicator_t	*null_indicators	= p->value;
	unsigned char			*data				= ((char *) p->value + IINQ_BITS_FOR_NULL(5));

	p->key = malloc((sizeof(char) * 3));

	if (NULL == p->key) {
		free(p->value);
		free(p);
		return NULL;
	}

	if (NULL != value_1) {
		strncpy((char *) p->key + 0, value_1, (sizeof(char) * 3));
	}

	if (NULL == value_1) {
		iinq_set_null_indicator(null_indicators, 1);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 1);
		strncpy(data, value_1, (sizeof(char) * 3));
		data += (sizeof(char) * 3);
	}

	if (NULL == value_2) {
		iinq_set_null_indicator(null_indicators, 2);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 2);
		strncpy(data, value_2, (sizeof(char) * 21));
		data += (sizeof(char) * 21);
	}

	if (NULL == value_3) {
		iinq_set_null_indicator(null_indicators, 3);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 3);
		strncpy(data, value_3, (sizeof(char) * 31));
		data += (sizeof(char) * 31);
	}

	if (NULL == value_4) {
		iinq_set_null_indicator(null_indicators, 4);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 4);
		*(int *) data	= NEUTRALIZE(value_4, int);
		data			+= sizeof(int);
	}

	if (NULL == value_5) {
		iinq_set_null_indicator(null_indicators, 5);
	}
	else {
		iinq_clear_null_indicator(null_indicators, 5);
		strncpy(data, value_5, (sizeof(char) * 31));
	}

	return p;
}
