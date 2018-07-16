#include "iinq_user_functions.h"

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

void
update(
	iinq_table_id	table_id,
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

	while ((status = iinq_next_record(cursor, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
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

	while ((status = iinq_next_record(cursor_temp, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		ion_boolean_t key_changed = boolean_false;

		memcpy(new_key, ion_record.key, dictionary.instance->record.key_size);

		for (i = 0; i < num_update; i++) {
			unsigned char *value;

			if (updates[i].implicit_field != 0) {
				int new_value;

				value = (char *) ion_record.value + calculateOffset(table_id, updates[i].implicit_field);

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

				value			= (char *) ion_record.value + calculateOffset(table_id, updates[i].update_field);
				*(int *) value	= new_value;
			}
			else {
				value = (char *) ion_record.value + calculateOffset(table_id, updates[i].update_field);

				if (getFieldType(table_id, updates[i].update_field) == iinq_int) {
					if (iinq_is_key_field(table_id, updates[i].update_field)) {
						*(int *) ((char *) new_key + iinq_calculate_key_offset(table_id, updates[i].update_field))	= NEUTRALIZE(updates[i].field_value, int);
						key_changed																					= boolean_true;
					}

					*(int *) value = NEUTRALIZE(updates[i].field_value, int);
				}
				else {
					size_t size = calculateOffset(table_id, updates[i].update_field + 1) - calculateOffset(table_id, updates[i].update_field);

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

			ion_record.value = malloc((sizeof(int) * 2) + (sizeof(char) * 83));

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

			ion_record.value = malloc((sizeof(int) * 1) + (sizeof(char) * 86));

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

		case 3:
			ion_record.key = malloc(sizeof(int) + sizeof(int));

			if (NULL == ion_record.key) {
				error = err_out_of_memory;
				goto END;
			}

			ion_record.value = malloc((sizeof(int) * 2) + (sizeof(char) * 6));

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

			ion_record.value = malloc((sizeof(int) * 2) + (sizeof(char) * 6));

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
delete_record(
	iinq_table_id	table_id,
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

	while ((status = iinq_next_record(cursor, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
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

	while ((status = iinq_next_record(cursor_temp, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
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
	p->value	= malloc((sizeof(int) * 2) + (sizeof(char) * 6) + IINQ_BITS_FOR_NULL(3));

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
	p->value	= malloc((sizeof(int) * 2) + (sizeof(char) * 6) + IINQ_BITS_FOR_NULL(3));

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

			ion_record.value = malloc((sizeof(int) * 2) + (sizeof(char) * 83));

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

			ion_record.value = malloc((sizeof(int) * 1) + (sizeof(char) * 86));

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

			ion_record.value = malloc((sizeof(int) * 2) + (sizeof(char) * 31));

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

			ion_record.value = malloc((sizeof(int) * 2) + (sizeof(char) * 6));

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

			ion_record.value = malloc((sizeof(int) * 2) + (sizeof(char) * 6));

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
	char	*value_3,
	int		*value_4,
	char	*value_5
) {
	iinq_prepared_sql *p = malloc(sizeof(iinq_prepared_sql));

	if (NULL == p) {
		return NULL;
	}

	p->table	= 0;
	p->value	= malloc((sizeof(int) * 2) + (sizeof(char) * 83) + IINQ_BITS_FOR_NULL(5));

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
	p->value	= malloc((sizeof(int) * 1) + (sizeof(char) * 86) + IINQ_BITS_FOR_NULL(5));

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

size_t
calculateOffset(
	iinq_table_id		tableId,
	iinq_field_num_t	field_num
) {
	switch (tableId) {
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
