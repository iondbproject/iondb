#include "iinq_testing_functions.h"

void
iinq_external_sort_destroy(
	iinq_query_operator_t **query_operator
) {
	if (NULL != *query_operator) {
		if (NULL != (*query_operator)->instance) {
			iinq_external_sort_t *external_sort = (iinq_external_sort_t *) (*query_operator)->instance;

			if (NULL != external_sort->cursor) {
				ion_external_sort_destroy_cursor(external_sort->cursor);
				free(external_sort->cursor);
			}

			if (NULL != external_sort->buffer) {
				free(external_sort->buffer);
			}

			if (NULL != external_sort->record_buf) {
				free(external_sort->record_buf);
			}

			if (NULL != external_sort->es) {
				if (NULL != external_sort->es->input_file) {
					fclose(external_sort->es->input_file);
					fremove("orderby");
				}

				if (NULL != external_sort->es->context) {
					if (NULL != ((iinq_sort_context_t *) external_sort->es->context)->parts) {
						free(((iinq_sort_context_t *) external_sort->es->context)->parts);
					}

					free(external_sort->es->context);
				}

				free(external_sort->es);
			}

			if (NULL != external_sort->super.field_info) {
				free(external_sort->super.field_info);
			}

			if (NULL != external_sort->super.fields) {
				free(external_sort->super.fields);
			}

			free(external_sort);
		}

		free(*query_operator);
		*query_operator = NULL;
	}
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

ion_err_t
iinq_execute_prepared(
	iinq_prepared_sql *p
) {
	switch (p->table) {
		case 0: {
			if (iinq_check_null_indicator(p->value, 1)) {
				return err_unable_to_insert;
			}
		}
	}

	return iinq_execute(&p->dictionary, p->key, p->value, p->operation_type);
}

ion_err_t
iinq_drop_table(
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

	iinq_query_operator_t *query_operator = malloc(sizeof(iinq_query_operator_t));

	if (NULL == query_operator) {
		input_operator->instance->destroy(&input_operator);
		return NULL;
	}

	query_operator->instance = malloc(sizeof(iinq_projection_t));

	if (NULL == query_operator->instance) {
		free(query_operator);
		input_operator->instance->destroy(&input_operator);
		return NULL;
	}

	query_operator->instance->destroy = iinq_projection_destroy;

	iinq_projection_t *projection = (iinq_projection_t *) query_operator->instance;

	projection->super.type						= iinq_projection_e;

	projection->super.input_operator			= input_operator;
	projection->super.parent_operator			= NULL;
	input_operator->instance->parent_operator	= query_operator;
	projection->super.num_fields				= num_fields;

	projection->input_field_nums				= malloc(sizeof(iinq_field_num_t) * num_fields);

	if (NULL == projection->input_field_nums) {
		free(projection);
		free(query_operator);
		input_operator->instance->destroy(&input_operator);
		return NULL;
	}

	memcpy(projection->input_field_nums, field_nums, sizeof(iinq_field_num_t) * num_fields);

	projection->super.null_indicators = malloc(IINQ_BITS_FOR_NULL(num_fields));

	if (NULL == projection->super.null_indicators) {
		free(projection->input_field_nums);
		free(projection);
		free(query_operator);
		input_operator->instance->destroy(&input_operator);
		return NULL;
	}

	projection->super.field_info = malloc(sizeof(iinq_field_info_t) * num_fields);

	if (NULL == projection->super.field_info) {
		free(projection->super.null_indicators);
		free(projection);
		free(query_operator);
		input_operator->instance->destroy(&input_operator);
		return NULL;
	}

	projection->super.fields = malloc(sizeof(ion_value_t) * num_fields);

	if (NULL == projection->super.fields) {
		free(projection->super.field_info);
		free(projection->super.null_indicators);
		free(projection);
		free(query_operator);
		input_operator->instance->destroy(&input_operator);
		return NULL;
	}

	int i;

	for (i = 0; i < num_fields; i++) {
		projection->super.field_info[i] = input_operator->instance->field_info[field_nums[i] - 1];
		projection->super.fields[i]		= input_operator->instance->fields[field_nums[i] - 1];
	}

	query_operator->status = ION_STATUS_OK(0);

	return query_operator;
}

void
iinq_projection_destroy(
	iinq_query_operator_t **query_operator
) {
	if (*query_operator != NULL) {
		if ((*query_operator)->instance != NULL) {
			iinq_projection_t *projection = (iinq_projection_t *) (*query_operator)->instance;

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

			if (NULL != projection->super.input_operator) {
				projection->super.input_operator->instance->destroy(&projection->super.input_operator);
			}

			free(projection);
		}

		free(*query_operator);
		*query_operator = NULL;
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
iinq_create_table(
	iinq_table_id_t		table_id,
	ion_key_type_t		keyType,
	ion_key_size_t		keySize,
	ion_value_size_t	value_size
) {
	ion_err_t error = iinq_create_source(table_id, keyType, keySize, value_size);

	return error;
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

iinq_query_operator_t *
iinq_dictionary_init(
	iinq_table_id_t			table_id,
	iinq_field_num_t		num_fields,
	ion_predicate_type_t	predicate_type,
	...
) {
	int							i;
	ion_err_t					error;
	iinq_dictionary_operator_t	*dictionary_operator;
	ion_dict_cursor_t			*cursor		= NULL;
	ion_record_t				*record		= NULL;
	ion_dictionary_t			*dictionary = NULL;
	ion_dictionary_handler_t	*handler	= NULL;

	iinq_query_operator_t *query_operator	= malloc(sizeof(iinq_query_operator_t));

	if (NULL == query_operator) {
		return NULL;
	}

	query_operator->instance = malloc(sizeof(iinq_dictionary_operator_t));

	if (NULL == query_operator->instance) {
		free(query_operator);
		return NULL;
	}

	dictionary_operator						= (iinq_dictionary_operator_t *) query_operator->instance;
	dictionary_operator->super.type			= iinq_dictionary_operator_e;
	dictionary_operator->super.num_fields	= num_fields;

	ion_predicate_t predicate;

	switch (predicate_type) {
		case predicate_all_records: {
			error = dictionary_build_predicate(&predicate, predicate_type);
			break;
		}

		case predicate_equality: {
			va_list arg_list;

			va_start(arg_list, predicate_type);
			error = dictionary_build_predicate(&predicate, predicate_type, va_arg(arg_list, ion_key_t));
			va_end(arg_list);
			break;
		}

		case predicate_range: {
			va_list arg_list;

			va_start(arg_list, predicate_type);
			error = dictionary_build_predicate(&predicate, predicate_type, va_arg(arg_list, ion_key_t), va_arg(arg_list, ion_key_t));
			va_end(arg_list);
			break;
		}

		case predicate_predicate: {
			error = dictionary_build_predicate(&predicate, predicate_type);
			break;
		}

		default:
			error = err_not_implemented;
	}

	if (err_ok != error) {
		free(query_operator->instance);
		free(query_operator);
		return NULL;
	}

	dictionary			= &dictionary_operator->dictionary;
	handler				= &dictionary_operator->handler;
	dictionary->handler = handler;
	record				= &dictionary_operator->record;

	error				= iinq_open_source(table_id, dictionary, handler);

	if (err_ok != error) {
		free(query_operator->instance);
		free(query_operator);
		return NULL;
	}

	error = dictionary_find(dictionary, &predicate, &cursor);

	if (err_ok != error) {
		if (NULL != cursor) {
			cursor->destroy(&cursor);
		}

		ion_close_dictionary(&dictionary);
		free(query_operator->instance);
		free(query_operator);
		return NULL;
	}

	dictionary_operator->cursor		= cursor;

	dictionary_operator->record.key = malloc(dictionary->instance->record.key_size);

	if (NULL == dictionary_operator->record.key) {
		cursor->destroy(&cursor);
		ion_close_dictionary(&dictionary);
		free(query_operator->instance);
		free(query_operator);
		return NULL;
	}

	record->value = malloc(dictionary->instance->record.value_size);

	if (NULL == record->value) {
		free(record->key);
		cursor->destroy(&cursor);
		ion_close_dictionary(&dictionary);
		free(query_operator->instance);
		free(query_operator);
		return NULL;
	}

	dictionary_operator->super.field_info = malloc(sizeof(iinq_field_info_t) * num_fields);

	if (NULL == dictionary_operator->super.field_info) {
		free(record->value);
		free(record->key);
		cursor->destroy(&cursor);
		ion_close_dictionary(&dictionary);
		free(query_operator->instance);
		free(query_operator);
		return NULL;
	}

	dictionary_operator->super.null_indicators	= dictionary_operator->record.value;

	dictionary_operator->super.fields			= malloc(sizeof(ion_value_t) * num_fields);

	if (NULL == dictionary_operator->super.fields) {
		free(dictionary_operator->super.field_info);
		free(record->value);
		free(record->key);
		cursor->destroy(&cursor);
		ion_close_dictionary(&dictionary);
		free(query_operator->instance);
		free(query_operator);
		return NULL;
	}

	for (i = 0; i < num_fields; i++) {
		dictionary_operator->super.field_info[i]	= (iinq_field_info_t) {
			table_id, i + 1
		};
		dictionary_operator->super.fields[i]		= (unsigned char *) record->value + iinq_calculate_offset(table_id, i + 1);
	}

	ion_close_master_table();

	query_operator->instance->destroy			= iinq_dictionary_operator_destroy;
	query_operator->instance->input_operator	= query_operator->instance->parent_operator = NULL;
	query_operator->status						= ION_STATUS_OK(0);

	return query_operator;
}

ion_boolean_t
iinq_next(
	iinq_result_set_t *result_set
) {
	iinq_query_operator_t *curr;

	do {
		curr = result_set->tail;

		while (NULL != curr) {
			if (curr->instance->type == iinq_dictionary_operator_e) {
				iinq_dictionary_operator_t *dict_op = (iinq_dictionary_operator_t *) curr->instance;

				if ((cs_cursor_active == dict_op->cursor->next(dict_op->cursor, &dict_op->record)) || (cs_cursor_initialized == dict_op->cursor->status)) {
					curr->status.count++;
				}
				else {
					result_set->status = ION_STATUS_ERROR(err_file_hit_eof);
					return boolean_false;
				}
			}
			else if (curr->instance->type == iinq_external_sort_e) {
				iinq_external_sort_t *external_sort = (iinq_external_sort_t *) curr->instance;

				if ((err_ok != external_sort->cursor->next(external_sort->cursor, external_sort->record_buf)) || (cs_cursor_active != external_sort->cursor->status)) {
					result_set->status = ION_STATUS_ERROR(err_file_hit_eof);
					return boolean_false;
				}

				curr->status.count++;
			}
			else if (curr->instance->type == iinq_projection_e) {
				int					i;
				iinq_projection_t	*projection = (iinq_projection_t *) curr->instance;

				for (i = 0; i < projection->super.num_fields; i++) {
					if (iinq_check_null_indicator(projection->super.input_operator->instance->null_indicators, projection->input_field_nums[i])) {
						iinq_set_null_indicator(projection->super.null_indicators, i + 1);
					}
					else {
						iinq_clear_null_indicator(projection->super.null_indicators, i + 1);
					}
				}

				curr->status.count++;
			}
			else {
				result_set->status = ION_STATUS_ERROR(err_illegal_state);
				return boolean_false;
			}

			curr = curr->instance->parent_operator;
		}
	} while (NULL != curr);

	return boolean_true;
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

iinq_query_operator_t *
iinq_external_sort_init(
	iinq_query_operator_t	*input_operator,
	int						num_orderby,
	iinq_order_by_field_t	*order_by_fields
) {
	if (NULL == input_operator) {
		return NULL;
	}

	iinq_size_t			total_orderby_size		= 0;
	iinq_field_num_t	num_fields				= input_operator->instance->num_fields;
	iinq_order_part_t	*orderby_order_parts	= malloc(sizeof(iinq_order_part_t) * num_orderby);

	if (NULL == orderby_order_parts) {
		input_operator->instance->destroy(&input_operator);
		return NULL;
	}

	int					i;
	ion_value_size_t	value_size = 0;

	for (i = 0; i < num_fields; i++) {
		iinq_table_id_t		table_id	= input_operator->instance->field_info[i].table_id;
		iinq_field_num_t	field_num	= input_operator->instance->field_info[i].field_num;

		value_size += iinq_calculate_offset(table_id, field_num + 1) - iinq_calculate_offset(table_id, field_num);
	}

	for (i = 0; i < num_orderby; i++) {
		orderby_order_parts[i].pointer = input_operator->instance->fields[order_by_fields[i].field_num - 1];

		iinq_field_num_t	field_num	= input_operator->instance->field_info[order_by_fields[i].field_num - 1].field_num;
		iinq_table_id_t		table_id	= input_operator->instance->field_info[order_by_fields[i].field_num - 1].table_id;

		orderby_order_parts[i].direction	= order_by_fields[i].direction;
		orderby_order_parts[i].size			= iinq_calculate_offset(table_id, field_num + 1) - iinq_calculate_offset(table_id, field_num);

		switch (iinq_get_field_type(table_id, field_num)) {
			case iinq_int:
				orderby_order_parts[i].type = IINQ_ORDERTYPE_INT;
				break;

			case iinq_unsigned_int:
				orderby_order_parts[i].type = IINQ_ORDERTYPE_UINT;
				break;

			case iinq_float:
				orderby_order_parts[i].type = IINQ_ORDERTYPE_FLOAT;
				break;

			case iinq_null_terminated_string:
			case iinq_char_array:
			default:
				orderby_order_parts[i].type = IINQ_ORDERTYPE_OTHER;
				break;
		}

		total_orderby_size += orderby_order_parts[i].size;
	}

	FILE *file						= fopen("orderby", "wb");

	int write_page_remaining		= IINQ_PAGE_SIZE;

	iinq_result_set_t *result_set	= iinq_init_result_set(input_operator);

	if (NULL == result_set) {
		return NULL;
	}

	while (iinq_next(result_set)) {
		if (write_page_remaining < (total_orderby_size + IINQ_BITS_FOR_NULL(num_fields) + value_size)) {
			char x = 0;

			for (i = 0; i < write_page_remaining; i++) {
				if (1 != fwrite(&x, 1, 1, file)) {
					break;
				}
			}

			write_page_remaining = IINQ_PAGE_SIZE;
		}

		for (i = 0; i < num_orderby; i++) {
			if (1 != fwrite(orderby_order_parts[i].pointer, orderby_order_parts[i].size, 1, file)) {
				break;
			}
			else {
				write_page_remaining -= orderby_order_parts[i].size;
			}
		}

		if (1 != fwrite(input_operator->instance->null_indicators, IINQ_BITS_FOR_NULL(num_fields), 1, file)) {
			break;
		}
		else {
			write_page_remaining -= IINQ_BITS_FOR_NULL(num_fields);
		}

		int j;

		for (j = 0; j < num_fields; j++) {
			iinq_field_num_t	field_num	= input_operator->instance->field_info[j].field_num;
			iinq_table_id_t		table_id	= input_operator->instance->field_info[j].table_id;
			size_t				field_size	= iinq_calculate_offset(table_id, field_num + 1) - iinq_calculate_offset(table_id, field_num);

			if (1 != fwrite(input_operator->instance->fields[j], field_size, 1, file)) {
				break;
			}
			else {
				write_page_remaining -= field_size;
			}
		}
	}

	/* All records have been written, so close file */
	if (NULL != file) {
		fclose(file);
	}

	iinq_query_operator_t *query_operator = malloc(sizeof(iinq_query_operator_t));

	query_operator->instance		= malloc(sizeof(iinq_external_sort_t));
	query_operator->instance->type	= iinq_external_sort_e;

	iinq_external_sort_t *external_sort = (iinq_external_sort_t *) query_operator->instance;

	ion_external_sort_t *es				= malloc(sizeof(ion_external_sort_t));

	external_sort->es	= es;

	/* Open file in read mode for sorting */
	file				= fopen("orderby", "rb");

	iinq_sort_context_t *context = malloc(sizeof(iinq_sort_context_t));

	context->parts	= orderby_order_parts;
	context->n		= num_orderby;

	ion_err_t error			= ion_external_sort_init(es, file, context, iinq_sort_compare, 0, IINQ_BITS_FOR_NULL(num_fields) + value_size + total_orderby_size, IINQ_PAGE_SIZE, boolean_false, ION_FILE_SORT_FLASH_MINSORT);

	uint16_t buffer_size	= ion_external_sort_bytes_of_memory_required(es, 0, boolean_false);

	char *buffer			= malloc(buffer_size);
	/* recordbuf needs enough room for the sort field and the table tuple (sort field is stored twice) */
	char *record_buf		= malloc((total_orderby_size + IINQ_BITS_FOR_NULL(num_fields) + value_size));

	query_operator->instance->null_indicators	= record_buf + total_orderby_size;

	query_operator->instance->num_fields		= num_fields;
	query_operator->instance->field_info		= malloc(sizeof(iinq_field_info_t) * num_fields);
	memcpy(query_operator->instance->field_info, input_operator->instance->field_info, sizeof(iinq_field_info_t) * num_fields);
	query_operator->instance->fields			= malloc(sizeof(ion_value_t) * num_fields);

	size_t offset = total_orderby_size + IINQ_BITS_FOR_NULL(num_fields);

	for (i = 0; i < num_fields; i++) {
		iinq_table_id_t		table_id	= query_operator->instance->field_info[i].table_id;
		iinq_field_num_t	field_num	= query_operator->instance->field_info[i].field_num;

		query_operator->instance->fields[i] = record_buf + offset;
		offset								+= iinq_calculate_offset(table_id, field_num + 1) - iinq_calculate_offset(table_id, field_num);
	}

	iinq_close_result_set(result_set);

	external_sort->super.input_operator = external_sort->super.parent_operator = NULL;

	external_sort->buffer				= buffer;

	external_sort->record_buf			= record_buf;

	ion_external_sort_cursor_t *cursor = malloc(sizeof(ion_external_sort_cursor_t));

	error								= ion_external_sort_init_cursor(es, cursor, buffer, buffer_size);
	external_sort->cursor				= cursor;

	query_operator->instance->destroy	= iinq_external_sort_destroy;

	return query_operator;
}

iinq_result_set_t *
iinq_init_result_set(
	iinq_query_operator_t *query_operator
) {
	if (NULL == query_operator) {
		return NULL;
	}

	iinq_result_set_t *result_set = malloc(sizeof(iinq_result_set_t));

	if (NULL == result_set) {
		query_operator->instance->destroy(query_operator);
		return NULL;
	}

	result_set->head = result_set->tail = query_operator;

	while (NULL != result_set->tail->instance->input_operator) {
		result_set->tail = result_set->tail->instance->input_operator;
	}

	result_set->status = ION_STATUS_OK(0);

	return result_set;
}

void
iinq_dictionary_operator_destroy(
	iinq_query_operator_t **query_operator
) {
	if (NULL != *query_operator) {
		if (NULL != (*query_operator)->instance) {
			iinq_dictionary_operator_t *dict_op = (iinq_dictionary_operator_t *) (*query_operator)->instance;

			if (NULL != dict_op->super.field_info) {
				free(dict_op->super.field_info);
			}

			if (NULL != dict_op->super.fields) {
				free(dict_op->super.fields);
			}

			if (NULL != dict_op->record.value) {
				free(dict_op->record.value);
			}

			if (NULL != dict_op->record.key) {
				free(dict_op->record.key);
			}

			if (NULL != dict_op->cursor) {
				dict_op->cursor->destroy(&dict_op->cursor);
			}

			ion_close_dictionary(&dict_op->dictionary);

			free(dict_op);
		}

		free(*query_operator);
		*query_operator = NULL;
	}
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

	p->operation_type = iinq_insert_t;

	ion_err_t error = iinq_open_source(0, &p->dictionary, &p->handler);

	if (err_ok != error) {
		free(p);
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
