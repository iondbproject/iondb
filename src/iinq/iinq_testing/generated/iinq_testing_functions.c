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

void
print_table(
	iinq_table_id tableId
) {
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler = &handler;

	ion_err_t error = iinq_open_source(tableId, &dictionary, &handler);

	if (err_ok != error) {
		printf("Print error: %d", error);
	}

	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(&dictionary, &predicate, &cursor);

	ion_cursor_status_t cursor_status;

	ion_record_t ion_record;

	switch (tableId) {
		case 0:
			ion_record.key		= malloc(sizeof(int));
			ion_record.value	= malloc((sizeof(int) * 2) + (sizeof(char) * 31));
			break;
	}

	unsigned char *value;

	switch (tableId) {
		case 0:

			while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				value	= ion_record.value;

				printf("%10d, ", NEUTRALIZE(value, int));
				value	+= sizeof(int);

				printf("%31s, ", (char *) value);
				value	+= (sizeof(char) * 31);

				printf("%10d\n", NEUTRALIZE(value, int));
			}

			printf("\n");
			break;
	}
}

void
setParam(
	iinq_prepared_sql	*p,
	iinq_field_num_t	field_num,
	ion_value_t			val
) {
	unsigned char *data = p->value;

	iinq_field_t type	= getFieldType(p->table, field_num);

	data += calculateOffset(p->table, field_num);

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
		ion_delete_dictionary((*result_set)->dictionary_ref.cursor->dictionary, (*result_set)->dictionary_ref.cursor->dictionary->instance->id);
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

void
drop_table(
	iinq_table_id *tableId
) {
	ion_err_t error;

	error = iinq_drop(tableId);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	printf("Table %d has been deleted.\n", tableId);
}

void
execute(
	iinq_prepared_sql *p
) {
	switch (p->table) {
		case 0: {
			iinq_execute(0, p->key, p->value, iinq_insert_t);
			break;
		}
	}
}

iinq_result_set *
iinq_select(
	iinq_table_id		table_id,
	size_t				project_size,
	int					num_wheres,
	iinq_field_num_t	num_fields,
	...
) {
	int					i;
	va_list				valist;
	iinq_where_params_t *where_list = NULL;

	va_start(valist, num_fields);

	ion_err_t		error;
	iinq_result_set *result_set = malloc(sizeof(iinq_result_set));
	ion_predicate_t *predicate	= &result_set->dictionary_ref.predicate;

	dictionary_build_predicate(predicate, predicate_all_records);

	ion_dictionary_t			*dictionary = &result_set->dictionary_ref.dictionary;
	ion_dictionary_handler_t	*handler	= &result_set->dictionary_ref.handler;

	dictionary->handler			= handler;
	result_set->status.error	= iinq_open_source(table_id, dictionary, handler);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(dictionary, predicate, &cursor);

	result_set->record.key				= malloc(dictionary->instance->record.key_size);
	result_set->record.value			= malloc(dictionary->instance->record.value_size);
	result_set->dictionary_ref.cursor	= cursor;
	result_set->table_id				= table_id;

	if (num_wheres > 0) {
		where_list			= va_arg(valist, iinq_where_params_t *);
		result_set->wheres	= where_list;
	}

	result_set->num_wheres = num_wheres;

	iinq_field_num_t *fields = va_arg(valist, iinq_field_num_t *);

	result_set->num_fields	= num_fields;
	result_set->offset		= malloc(sizeof(unsigned int) * num_fields);
	result_set->fields		= fields;

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

void
create_table(
	iinq_table_id		tableId,
	ion_key_type_t		keyType,
	ion_key_size_t		keySize,
	ion_value_size_t	project_size
) {
	ion_err_t error = iinq_create_source(tableId, keyType, keySize, project_size);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}
}

iinq_prepared_sql *
iinq_insert_0(
	int		value_1,
	char	*value_2,
	int		value_3
) {
	iinq_prepared_sql *p = malloc(sizeof(iinq_prepared_sql));

	p->table	= 0;
	p->value	= malloc((sizeof(int) * 2) + (sizeof(char) * 31));

	unsigned char *data = p->value;

	p->key					= malloc(sizeof(int));
	*(int *) (p->key + 0)	= value_1;

	*(int *) data			= value_1;
	data					+= sizeof(int);

	strncpy(data, value_2, (sizeof(char) * 31));
	data					+= (sizeof(char) * 31);

	*(int *) data			= value_3;

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
					return 0;

				case 2:
					return sizeof(int);

				case 3:
					return sizeof(int) + (sizeof(char) * 31);

				case 4:
					return sizeof(int) + (sizeof(char) * 31) + sizeof(int);

				default:
					return 0;
			}
		}

		default:
			return 0;
	}
}
