#include "iinq_testing_functions.h"

unsigned int
iinq_calculate_key_offset(
	iinq_table_id		table_id,
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

ion_boolean_t
iinq_table_scan_next(
	iinq_result_set *select
) {
	while (cs_cursor_active == select->dictionary_ref.cursor->next(select->dictionary_ref.cursor, &select->record) || cs_cursor_initialized == select->dictionary_ref.cursor->status) {
		if (!where(select->table_id, &select->record, select->num_wheres, select->wheres)) {
			continue;
		}

		return boolean_true;
	}

	return boolean_false;
}

iinq_field_t
getFieldType(
	iinq_table_id		table,
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

void
iinq_destroy_table_scan(
	iinq_result_set **result_set
) {
	if (NULL != (*result_set)->record.key) {
		free((*result_set)->record.key);
		(*result_set)->record.key = NULL;
	}

	if (NULL != (*result_set)->record.value) {
		free((*result_set)->record.value);
		(*result_set)->record.value = NULL;
	}

	if ((*result_set)->dictionary_ref.temp_dictionary) {
		ion_init_master_table();
		ion_delete_dictionary(&(*result_set)->dictionary_ref.dictionary, (*result_set)->dictionary_ref.dictionary.instance->id);
	}
	else {
		ion_close_dictionary(&(*result_set)->dictionary_ref.dictionary);
	}

	if (NULL != (*result_set)->dictionary_ref.cursor) {
		(*result_set)->dictionary_ref.cursor->destroy(&(*result_set)->dictionary_ref.cursor);
	}

	if (NULL != (*result_set)->offset) {
		free((*result_set)->offset);
		(*result_set)->offset = NULL;
	}

	free(*result_set);
	*result_set = NULL;
}

ion_err_t
iinq_print_keys(
	iinq_table_id table_id
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

			ion_record.value = malloc((sizeof(int) * 2) + (sizeof(char) * 31));

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

iinq_result_set *
iinq_table_scan_init(
	iinq_table_id		table_id,
	int					num_wheres,
	iinq_field_num_t	num_fields,
	...
) {
	int					i;
	va_list				valist;
	iinq_where_params_t *where_list = NULL;

	va_start(valist, num_fields);

	iinq_result_set *result_set = malloc(sizeof(iinq_result_set));

	if (NULL == result_set) {
		return NULL;
	}

	ion_predicate_t *predicate = &result_set->dictionary_ref.predicate;

	result_set->status.error = dictionary_build_predicate(predicate, predicate_all_records);

	if (err_ok != result_set->status.error) {
		return result_set;
	}

	ion_dictionary_t			*dictionary = &result_set->dictionary_ref.dictionary;
	ion_dictionary_handler_t	*handler	= &result_set->dictionary_ref.handler;

	dictionary->handler			= handler;
	result_set->status.error	= iinq_open_source(table_id, dictionary, handler);

	if (err_ok != result_set->status.error) {
		return result_set;
	}

	ion_dict_cursor_t *cursor = NULL;

	result_set->status.error = dictionary_find(dictionary, predicate, &cursor);

	if (err_ok != result_set->status.error) {
		return result_set;
	}

	result_set->record.key = malloc(dictionary->instance->record.key_size);

	if (NULL == result_set->record.key) {
		result_set->status.error = err_out_of_memory;
		return result_set;
	}

	result_set->record.value = malloc(dictionary->instance->record.value_size);

	if (NULL == result_set->record.value) {
		result_set->status.error = err_out_of_memory;
		free(result_set->record.key);
		return result_set;
	}

	result_set->dictionary_ref.cursor	= cursor;
	result_set->table_id				= table_id;

	if (num_wheres > 0) {
		where_list			= va_arg(valist, iinq_where_params_t *);
		result_set->wheres	= where_list;
	}
	else {
		result_set->wheres = NULL;
	}

	result_set->num_wheres = num_wheres;

	iinq_field_num_t *fields = va_arg(valist, iinq_field_num_t *);

	result_set->num_fields	= num_fields;
	result_set->offset		= malloc(sizeof(unsigned int) * num_fields);

	if (NULL == result_set->offset) {
		free(result_set->record.value);
		free(result_set->record.key);
		result_set->status.error = err_out_of_memory;
		return result_set;
	}

	result_set->fields = fields;

	for (i = 0; i < num_fields; i++) {
		result_set->offset[i] = calculateOffset(table_id, fields[i]);
	}

	va_end(valist);

	ion_close_master_table();
	result_set->next							= iinq_table_scan_next;
	result_set->destroy							= iinq_destroy_table_scan;
	result_set->dictionary_ref.temp_dictionary	= boolean_false;

	return result_set;
}

ion_err_t
drop_table(
	iinq_table_id table_id
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

ion_err_t
execute(
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

void
iinq_set_param(
	iinq_prepared_sql	*p,
	iinq_field_num_t	field_num,
	ion_value_t			val
) {
	iinq_null_indicator_t	*null_indicators	= p->value;
	unsigned char			*data				= ((char *) p->value) + calculateOffset(p->table, field_num);

	if (NULL == val) {
		iinq_set_null_indicator(null_indicators, field_num);
	}
	else {
		iinq_clear_null_indicator(null_indicators, field_num);

		iinq_field_t type = getFieldType(p->table, field_num);

		if (type == iinq_int) {
			if (iinq_is_key_field(p->table, field_num)) {
				*(int *) (p->key + iinq_calculate_key_offset(p->table, field_num)) = NEUTRALIZE(val, int);
			}

			*(int *) data = NEUTRALIZE(val, int);
		}
		else {
			size_t size = calculateOffset(p->table, field_num + 1) - calculateOffset(p->table, field_num);

			if (iinq_is_key_field(p->table, field_num)) {
				strncpy(p->key + iinq_calculate_key_offset(p->table, field_num), val, size);
			}

			strncpy(data, val, size);
		}
	}
}

ion_boolean_t
iinq_is_key_field(
	iinq_table_id		table_id,
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

ion_err_t
iinq_print_table(
	iinq_table_id tableId
) {
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	ion_cursor_status_t cursor_status;

	ion_record_t ion_record;

	ion_record.key		= NULL;
	ion_record.value	= NULL;

	ion_dict_cursor_t *cursor = NULL;

	dictionary.handler = &handler;

	ion_err_t error = iinq_open_source(tableId, &dictionary, &handler);

	if (err_ok != error) {
		return error;
	}

	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	dictionary_find(&dictionary, &predicate, &cursor);

	switch (tableId) {
		case 0:
			ion_record.key = malloc(sizeof(int));

			if (NULL == ion_record.key) {
				error = err_out_of_memory;
				goto END;
			}

			ion_record.value = malloc((sizeof(int) * 2) + (sizeof(char) * 31));

			if (NULL == ion_record.value) {
				error = err_out_of_memory;
				goto END;
			}

			break;
	}

	unsigned char			*value;
	iinq_null_indicator_t	*null_indicators;

	switch (tableId) {
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
	iinq_table_id		tableId,
	ion_key_type_t		keyType,
	ion_key_size_t		keySize,
	ion_value_size_t	value_size
) {
	ion_err_t error = iinq_create_source(tableId, keyType, keySize, value_size);

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
	p->value	= malloc((sizeof(int) * 2) + (sizeof(char) * 31) + IINQ_BITS_FOR_NULL(3));

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

size_t
calculateOffset(
	iinq_table_id		tableId,
	iinq_field_num_t	field_num
) {
	switch (tableId) {
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
