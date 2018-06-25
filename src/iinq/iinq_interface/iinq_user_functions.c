#include "iinq_user_functions.h"

ion_boolean_t
next(
	iinq_result_set *select
) {
	if (select->status.count < select->num_recs - 1) {
		select->status.count++;
		return boolean_true;
	}

	select->status.error = ion_init_master_table();

	if (err_ok != select->status.error) {
		return boolean_false;
	}

	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler		= &handler;
	select->status.error	= ion_open_dictionary(&handler, &dictionary, select->id);

	if (err_ok != select->status.error) {
		return boolean_false;
	}

	select->status.error = ion_delete_dictionary(&dictionary, select->id);

	if (err_ok != select->status.error) {
		return boolean_false;
	}

	select->status.error = ion_close_master_table();

	if (err_ok != select->status.error) {
		return boolean_false;
	}

	free(select->value);
	free(select->fields);
	return boolean_false;
}

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
	iinq_prepared_sql	p,
	iinq_field_num_t	field_num,
	ion_value_t			val
) {
	unsigned char *data = p.value;

	iinq_field_t type	= getFieldType(p.table, field_num);

	data += calculateOffset(p.table, field_num);

	if (type == iinq_int) {
		if (iinq_is_key_field(p.table, field_num)) {
			*(int *) (p.key + iinq_calculate_key_offset(p.table, field_num)) = NEUTRALIZE(val, int);
		}

		*(int *) data = NEUTRALIZE(val, int);
	}
	else {
		size_t size = calculateOffset(p.table, field_num + 1) - calculateOffset(p.table, field_num);

		if (iinq_is_key_field(p.table, field_num)) {
			strncpy(p.key + iinq_calculate_key_offset(p.table, field_num), val, size);
		}

		strncpy(data, val, size);
	}
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

char *
getString(
	iinq_result_set *select,
	int				field_num
) {
	iinq_field_num_t i, count = 0;

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler		= &handler;

	select->status.error	= ion_init_master_table();

	if (err_ok != select->status.error) {
		return "";
	}

	select->status.error = ion_open_dictionary(&handler, &dictionary, select->id);

	if (err_ok != select->status.error) {
		return "";
	}

	select->status.error = ion_close_master_table();

	if (err_ok != select->status.error) {
		return "";
	}

	dictionary_get(&dictionary, IONIZE(select->status.count, int), select->value);

	error = ion_close_dictionary(&dictionary);

	if (err_ok != select->status.error) {
		return "";
	}

	return (char *) (select->value + select->offset[field_num - 1]);
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
	iinq_prepared_sql p
) {
	switch (p.table) {
		case 0: {
			iinq_execute(0, p.key, p.value, iinq_insert_t);
			break;
		}
	}

	free(p.value);
	free(p.key);
}

iinq_result_set *
iinq_select(
	iinq_table_id		table_id,
	size_t				project_size,
	int					numWheres,
	iinq_field_num_t	num_fields,
	...
) {
	int					i;
	va_list				valist;
	iinq_where_params_t *where_list = NULL;

	va_start(valist, num_fields);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler = &handler;

	iinq_result_set *select = malloc(sizeof(iinq_result_set));

	select->status.error = iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != select->status.error) {
		return select;
	}

	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(&dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(dictionary.instance->record.key_size);
	ion_record.value	= malloc(dictionary.instance->record.value_size);

	ion_cursor_status_t status;

	int				count = 0;
	ion_boolean_t	condition_satisfied;

	if (numWheres > 0) {
		where_list = va_arg(valist, iinq_where_params_t *);
	}

	iinq_field_num_t *fields = va_arg(valist, iinq_field_num_t *);

	select->num_fields	= num_fields;
	select->fields		= malloc(sizeof(iinq_field_num_t) * num_fields);
	select->offset		= malloc(sizeof(unsigned int) * num_fields);
	memcpy(select->fields, fields, sizeof(iinq_field_num_t) * num_fields);

	unsigned int offset = 0;

	for (int i = 0; i < num_fields; i++) {
		select->offset[i]	= offset;
		offset				+= calculateOffset(table_id, fields[i] + 1) - calculateOffset(table_id, fields[i]);
	}

	va_end(valist);

	ion_dictionary_handler_t	handler_temp;
	ion_dictionary_t			dictionary_temp;

	select->status.error	= ion_init_master_table();
	iinq_select_handler_init(&handler_temp);
	dictionary_temp.handler = &handler_temp;

	select->status.error	= ion_master_table_create_dictionary(&handler_temp, &dictionary_temp, key_type_numeric_unsigned, sizeof(unsigned int), project_size, 10);

	if (err_ok != select->status.error) {
		return select;
	}

	ion_close_master_table();
	select->id = dictionary_temp.instance->id;

	while ((status = iinq_next_record(cursor, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		condition_satisfied = where(table_id, &ion_record, numWheres, where_list);

		if (condition_satisfied) {
			ion_value_t fieldlist	= malloc(project_size);
			ion_value_t data		= fieldlist;

			for (i = 0; i < num_fields; i++) {
				if (getFieldType(table_id, fields[i]) == iinq_int) {
					*(int *) data	= NEUTRALIZE(ion_record.value + calculateOffset(table_id, fields[i]), int);
					data			+= sizeof(int);
				}
				else {
					strncpy(data, ion_record.value + calculateOffset(table_id, fields[i]), calculateOffset(table_id, fields[i] + 1) - calculateOffset(table_id, fields[i]));
					data += calculateOffset(table_id, fields[i] + 1) - calculateOffset(table_id, fields[i]);
				}
			}

			select->status.error = dictionary_insert(&dictionary_temp, IONIZE(count, unsigned int), fieldlist).error;

			if (err_ok != select->status.error) {
				return select;
			}

			count++;
			free(fieldlist);
		}
	}

	cursor->destroy(&cursor);

	select->status.error = ion_close_dictionary(&dictionary);

	if (err_ok != select->status.error) {
		return select;
	}

	select->status.error = ion_close_dictionary(&dictionary_temp);

	if (err_ok != select->status.error) {
		return select;
	}

	select->num_recs		= count;
	select->status.count	= -1;
	select->table_id		= table_id;
	select->value			= malloc(project_size);
	free(ion_record.key);
	free(ion_record.value);

	return select;
}

int
getInt(
	iinq_result_set *select,
	int				field_num
) {
	iinq_field_num_t i;

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler		= &handler;

	select->status.error	= ion_init_master_table();

	if (err_ok != select->status.error) {
		return -1;
	}

	select->status.error = ion_open_dictionary(&handler, &dictionary, select->id);

	if (err_ok != select->status.error) {
		return -1;
	}

	select->status.error	= ion_close_master_table();
	dictionary_get(&dictionary, IONIZE(select->status.count, int), select->value);

	select->status.error	= ion_close_dictionary(&dictionary);

	if (err_ok != select->status.error) {
		return -1;
	}

	return NEUTRALIZE(select->value + select->offset[field_num - 1], int);
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

iinq_prepared_sql
iinq_insert_0(
	int		value_1,
	char	*value_2,
	int		value_3
) {
	iinq_prepared_sql p = { 0 };

	p.table = 0;
	p.value = malloc((sizeof(int) * 2) + (sizeof(char) * 31));

	unsigned char *data = p.value;

	p.key					= malloc(sizeof(int));
	*(int *) (p.key + 0)	= value_1;

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
