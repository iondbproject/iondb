#include "iinq_user_functions.h"

iinq_prepared_sql
insert_1(
	char	*value_1,
	char	*value_2,
	char	*value_3,
	int		value_4,
	char	*value_5
) {
	iinq_prepared_sql p = { 0 };

	p.table				= malloc(sizeof(int));
	*(int *) p.table	= 1;
	p.value				= malloc((sizeof(int) * 1) + (sizeof(char) * 82));

	unsigned char *data = p.value;

	p.key = malloc((sizeof(char) * 2));
	memcpy(p.key, value_1, 12);

	strcpy(data, value_1);
	data			+= (sizeof(char) * 2);

	strcpy(data, value_2);
	data			+= (sizeof(char) * 20);

	strcpy(data, value_3);
	data			+= (sizeof(char) * 30);

	*(int *) data	= value_4;
	data			+= sizeof(int);

	strcpy(data, value_5);

	return p;
}

ion_boolean_t
next(
	iinq_result_set *select
) {
	if (*(int *) select->count < (*(int *) select->num_recs) - 1) {
		*(int *) select->count = (*(int *) select->count) + 1;
		return boolean_true;
	}

	ion_err_t error = iinq_drop(255);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
	}

	free(select->value);
	free(select->fields);
	free(select->count);
	free(select->table_id);
	free(select->num_recs);
	free(select->num_fields);
	return boolean_false;
}

iinq_prepared_sql
insert_0(
	int		value_1,
	char	*value_2,
	char	*value_3,
	int		value_4,
	char	*value_5
) {
	iinq_prepared_sql p = { 0 };

	p.table				= malloc(sizeof(int));
	*(int *) p.table	= 0;
	p.value				= malloc((sizeof(int) * 2) + (sizeof(char) * 80));

	unsigned char *data = p.value;

	p.key			= malloc(sizeof(int));
	*(int *) p.key	= value_1;

	*(int *) data	= value_1;
	data			+= sizeof(int);

	strcpy(data, value_2);
	data			+= (sizeof(char) * 20);

	strcpy(data, value_3);
	data			+= (sizeof(char) * 30);

	*(int *) data	= value_4;
	data			+= sizeof(int);

	strcpy(data, value_5);

	return p;
}

iinq_prepared_sql
insert_2(
	int		value_1,
	char	*value_2,
	int		value_3
) {
	iinq_prepared_sql p = { 0 };

	p.table				= malloc(sizeof(int));
	*(int *) p.table	= 2;
	p.value				= malloc((sizeof(int) * 2) + (sizeof(char) * 30));

	unsigned char *data = p.value;

	p.key			= malloc(sizeof(int));
	*(int *) p.key	= value_1;

	*(int *) data	= value_1;
	data			+= sizeof(int);

	strcpy(data, value_2);
	data			+= (sizeof(char) * 30);

	*(int *) data	= value_3;

	return p;
}

void
print_table(
	iinq_table_id table_id
) {
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler = &handler;

	ion_err_t error = iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != error) {
		printf("Print error: %d", error);
	}

	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(&dictionary, &predicate, &cursor);

	ion_cursor_status_t cursor_status;

	ion_record_t ion_record;

	switch (table_id) {
		case 0:
			ion_record.key		= malloc(sizeof(int));
			ion_record.value	= malloc((sizeof(int) * 2) + (sizeof(char) * 80));
			break;

		case 1:
			ion_record.key		= malloc((sizeof(char) * 2));
			ion_record.value	= malloc((sizeof(int) * 1) + (sizeof(char) * 82));
			break;

		case 2:
			ion_record.key		= malloc(sizeof(int));
			ion_record.value	= malloc((sizeof(int) * 2) + (sizeof(char) * 30));
			break;
	}

	unsigned char *value;

	switch (table_id) {
		case 0:

			while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				value	= ion_record.value;

				printf("%10d, ", NEUTRALIZE(value, int));
				value	+= sizeof(int);

				printf("%20s, ", (char *) value);
				value	+= (sizeof(char) * 20);

				printf("%30s, ", (char *) value);
				value	+= (sizeof(char) * 30);

				printf("%10d, ", NEUTRALIZE(value, int));
				value	+= sizeof(int);

				printf("%30s\n", (char *) value);
			}

			printf("\n");
			break;

		case 1:

			while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				value	= ion_record.value;

				printf("%2s, ", (char *) value);
				value	+= (sizeof(char) * 2);

				printf("%20s, ", (char *) value);
				value	+= (sizeof(char) * 20);

				printf("%30s, ", (char *) value);
				value	+= (sizeof(char) * 30);

				printf("%10d, ", NEUTRALIZE(value, int));
				value	+= sizeof(int);

				printf("%30s\n", (char *) value);
			}

			printf("\n");
			break;

		case 2:

			while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
				value	= ion_record.value;

				printf("%10d, ", NEUTRALIZE(value, int));
				value	+= sizeof(int);

				printf("%30s, ", (char *) value);
				value	+= (sizeof(char) * 30);

				printf("%10d\n", NEUTRALIZE(value, int));
			}

			printf("\n");
			break;
	}
}

void
setParam(
	iinq_prepared_sql	p,
	int					field_num,
	void				*val
) {
	unsigned char *data = p.value;

	iinq_field_t type	= getFieldType(*p.table, field_num);

	data += calculateOffset(*p.table, field_num);

	if (type == iinq_int) {
		*(int *) data = (int) val;
	}
	else {
		strncpy(data, val, (calculateOffset(*p.table, field_num + 1) - calculateOffset(*p.table, field_num)));
	}
}

iinq_field_t
getFieldType(
	iinq_table_id	table,
	int				field_num
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

		default:
			return 0;
	}
}

void
update(
	iinq_table_id	table_id,
	ion_key_type_t	key_type,
	size_t			key_size,
	size_t			value_size,
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

	ion_record.key		= malloc(key_size);
	ion_record.value	= malloc(value_size);

	ion_cursor_status_t status;

	error = iinq_create_source(255, key_type, (ion_key_size_t) key_size, (ion_value_size_t) value_size);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
	}

	ion_dictionary_t			dictionary_temp;
	ion_dictionary_handler_t	handler_temp;

	dictionary_temp.handler = &handler_temp;

	error					= iinq_open_source(255, &dictionary_temp, &handler_temp);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
	}

	ion_boolean_t condition_satisfied;

	while ((status = iinq_next_record(cursor, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		condition_satisfied = where(table_id, &ion_record, num_wheres, &valist);

		if (condition_satisfied) {
			error = dictionary_insert(&dictionary_temp, ion_record.key, ion_record.value).error;

			if (err_ok != error) {
				printf("Error occurred. Error code: %i\n", error);
			}
		}
	}

	cursor->destroy(&cursor);

	int i;

	iinq_update_params_t updates[num_update];

	for (i = 0; i < num_wheres; i++) {
		va_arg(valist, void *);
	}

	for (i = 0; i < num_update; i++) {
		updates[i] = va_arg(valist, iinq_update_params_t);
	}

	va_end(valist);

	ion_predicate_t predicate_temp;

	dictionary_build_predicate(&predicate_temp, predicate_all_records);

	ion_dict_cursor_t *cursor_temp = NULL;

	dictionary_find(&dictionary_temp, &predicate_temp, &cursor_temp);

	while ((status = iinq_next_record(cursor_temp, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		for (i = 0; i < num_update; i++) {
			unsigned char *value;

			if (updates[i].implicit_field != 0) {
				int new_value;

				value = ion_record.value + calculateOffset(table_id, updates[i].implicit_field);

				switch (updates[i].operator) {
					case iinq_add:
						new_value = (NEUTRALIZE(value, int) + (int) updates[i].field_value);
						break;

					case iinq_subtract:
						new_value = (NEUTRALIZE(value, int) - (int) updates[i].field_value);
						break;

					case iinq_multiply:
						new_value = (NEUTRALIZE(value, int) * (int) updates[i].field_value);
						break;

					case iinq_divide:
						new_value = (NEUTRALIZE(value, int) / (int) updates[i].field_value);
						break;
				}

				value			= ion_record.value + calculateOffset(table_id, updates[i].update_field);
				*(int *) value	= new_value;
			}
			else {
				value = ion_record.value + calculateOffset(table_id, updates[i].update_field);

				if (getFieldType(table_id, updates[i].update_field) == iinq_int) {
					*(int *) value = (int) updates[i].field_value;
				}
				else {
					memcpy(value, updates[i].field_value, calculateOffset(table_id, updates[i].update_field) - calculateOffset(table_id, updates[i].update_field - 1));
				}
			}
		}

		error = dictionary_update(&dictionary, ion_record.key, ion_record.value).error;

		if (err_ok != error) {
			printf("Error occurred. Error code: %i\n", error);
		}
	}

	cursor_temp->destroy(&cursor_temp);
	iinq_drop(255);
	free(ion_record.key);
	free(ion_record.value);
}

char *
getString(
	iinq_result_set *select,
	int				field_num
) {
	int i, count = 0;

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler		= &handler;

	select->status.error	= iinq_open_source(255, &dictionary, &handler);

	if (err_ok != select->status.error) {
		return "";
	}

	dictionary_get(&dictionary, select->count, select->value);

	error = ion_close_dictionary(&dictionary);

	if (err_ok != select->status.error) {
		return "";
	}

	for (i = 0; i < *(int *) select->num_fields; i++) {
		int field = *(int *) (select->fields + sizeof(int) * i);

		if (getFieldType(select->table_id, field) == iinq_null_terminated_string) {
			count++;
		}

		if (count == field_num) {
			return (char *) (select->value + calculateOffset(select->table_id, field - 1));
		}
	}

	return "";
}

void
drop_table(
	iinq_table_id *table_id
) {
	ion_err_t error;

	error = iinq_drop(table_id);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	printf("Table %d has been deleted.\n", table_id);
}

void
execute(
	iinq_prepared_sql p
) {
	switch (*(int *) p.table) {
		case 0: {
			iinq_execute(0, IONIZE(*(int *) p.key, int), p.value, iinq_insert_t);
			break;
		}

		case 1: {
			iinq_execute(1, p.key, p.value, iinq_insert_t);
			break;
		}

		case 2: {
			iinq_execute(2, IONIZE(*(int *) p.key, int), p.value, iinq_insert_t);
			break;
		}
	}

	free(p.value);
	free(p.table);
	free(p.key);
}

void
delete_record(
	iinq_table_id	table_id,
	ion_key_type_t	key_type,
	size_t			key_size,
	size_t			value_size,
	int				num_fields,
	...
) {
	va_list valist;

	va_start(valist, num_fields);

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

	ion_record.key		= malloc(key_size);
	ion_record.value	= malloc(value_size);

	ion_cursor_status_t status;

	error = iinq_create_source(255, key_type, (ion_key_size_t) key_size, (ion_value_size_t) sizeof(int));

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	ion_dictionary_t			dictionary_temp;
	ion_dictionary_handler_t	handler_temp;

	dictionary_temp.handler = &handler_temp;

	error					= iinq_open_source(255, &dictionary_temp, &handler_temp);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	ion_boolean_t condition_satisfied;

	while ((status = iinq_next_record(cursor, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		condition_satisfied = where(table_id, &ion_record, num_fields, &valist);

		if (condition_satisfied) {
			error = dictionary_insert(&dictionary_temp, ion_record.key, IONIZE(0, int)).error;

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
	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	iinq_drop(255);
	free(ion_record.key);
	free(ion_record.value);
}

iinq_result_set
iinq_select(
	iinq_table_id	table_id,
	ion_key_type_t	key_type,
	size_t			key_size,
	size_t			value_size,
	int				num_wheres,
	int				num_fields,
	...
) {
	int		i;
	va_list valist, where_list;

	va_start(valist, num_fields);
	va_copy(where_list, valist);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler = &handler;

	iinq_result_set select = { 0 };

	select.status.error = iinq_open_source(table_id, &dictionary, &handler);

	if (err_ok != select.status.error) {
		return select;
	}

	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(&dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(key_size);
	ion_record.value	= malloc(value_size);

	ion_cursor_status_t status;

	int				count = 0;
	ion_boolean_t	condition_satisfied;

	int fields[num_fields];

	for (i = 0; i < num_wheres; i++) {
		va_arg(valist, void *);
	}

	select.num_fields			= malloc(sizeof(int));
	*(int *) select.num_fields	= num_fields;
	select.fields				= malloc(sizeof(int) * num_fields);

	unsigned char *field_list = select.fields;

	select.num_recs = malloc(sizeof(int));

	for (i = 0; i < num_fields; i++) {
		fields[i]			= va_arg(valist, int);

		*(int *) field_list = fields[i];

		if (i < num_fields - 1) {
			field_list += sizeof(int);
		}
	}

	va_end(valist);

	ion_dictionary_handler_t	handler_temp;
	ion_dictionary_t			dictionary_temp;

	select.status.error = iinq_create_source(255, key_type, (ion_key_size_t) key_size, (ion_value_size_t) value_size);

	if (err_ok != select.status.error) {
		return select;
	}

	dictionary_temp.handler = &handler_temp;

	select.status.error		= iinq_open_source(255, &dictionary_temp, &handler_temp);

	if (err_ok != select.status.error) {
		return select;
	}

	while ((status = iinq_next_record(cursor, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		condition_satisfied = where(table_id, &ion_record, num_wheres, &where_list);

		if (!condition_satisfied || (num_wheres == 0)) {
			unsigned char	*fieldlist	= malloc(value_size);
			unsigned char	*data		= fieldlist;

			for (i = 0; i < num_fields; i++) {
				if (getFieldType(table_id, fields[i]) == iinq_int) {
					*(int *) data	= NEUTRALIZE(ion_record.value + calculateOffset(table_id, fields[i] - 1), int);
					data			+= sizeof(int);
				}
				else {
					memcpy(data, ion_record.value + calculateOffset(table_id, fields[i] - 1), calculateOffset(table_id, fields[i]) - calculateOffset(table_id, fields[i] - 1));
					data += calculateOffset(table_id, fields[i]) - calculateOffset(table_id, fields[i] - 1);
				}
			}

			select.status.error = dictionary_insert(&dictionary_temp, IONIZE(count, int), fieldlist).error;

			if (err_ok != select.status.error) {
				return select;
			}

			count++;
			free(fieldlist);
		}
	}

	cursor->destroy(&cursor);

	select.status.error = ion_close_dictionary(&dictionary);

	if (err_ok != select.status.error) {
		return select;
	}

	select.status.error = ion_close_dictionary(&dictionary_temp);

	if (err_ok != select.status.error) {
		return select;
	}

	*(int *) select.num_recs			= count;
	select.table_id						= malloc(sizeof(int));
	*(iinq_table_id *) select.table_id	= table_id;
	select.value						= malloc(value_size);
	select.count						= malloc(sizeof(int));
	*(int *) select.count				= -1;

	free(ion_record.key);
	free(ion_record.value);

	return select;
}

int
getInt(
	iinq_result_set *select,
	int				field_num
) {
	int i, count = 0;

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler		= &handler;

	select->status.error	= iinq_open_source(255, &dictionary, &handler);

	if (err_ok != select->status.error) {
		return -1;
	}

	dictionary_get(&dictionary, select->count, select->value);

	select->status.error = ion_close_dictionary(&dictionary);

	if (err_ok != select->status.error) {
		return -1;
	}

	for (i = 0; i < *(int *) select->num_fields; i++) {
		int field = *(int *) (select->fields + sizeof(int) * i);

		if (getFieldType(select->table_id, field) == iinq_int) {
			count++;
		}

		if (count == field_num) {
			return NEUTRALIZE(select->value + calculateOffset(select->table_id, field - 1), int);
		}
	}

	return 0;
}

void
create_table(
	iinq_table_id		table_id,
	ion_key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size
) {
	ion_err_t error = iinq_create_source(table_id, key_type, key_size, value_size);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}
}

size_t
calculateOffset(
	iinq_table_id	table_id,
	int				field_num
) {
	switch (table_id) {
		case 0: {
			switch (field_num) {
				case 1:
					return 0;

				case 2:
					return sizeof(int);

				case 3:
					return sizeof(int) + (sizeof(char) * 20);

				case 4:
					return sizeof(int) + (sizeof(char) * 20) + (sizeof(char) * 30);

				case 5:
					return sizeof(int) + (sizeof(char) * 20) + (sizeof(char) * 30) + sizeof(int);

				default:
					return 0;
			}
		}

		case 1: {
			switch (field_num) {
				case 1:
					return 0;

				case 2:
					return sizeof(char) * 2;

				case 3:
					return (sizeof(char) * 2) + (sizeof(char) * 20);

				case 4:
					return (sizeof(char) * 2) + (sizeof(char) * 20) + (sizeof(char) * 30);

				case 5:
					return (sizeof(char) * 2) + (sizeof(char) * 20) + (sizeof(char) * 30) + sizeof(int);

				default:
					return 0;
			}
		}

		case 2: {
			switch (field_num) {
				case 1:
					return 0;

				case 2:
					return sizeof(int);

				case 3:
					return sizeof(int) + (sizeof(char) * 30);

				default:
					return 0;
			}
		}

		default:
			return 0;
	}
}
