#include "iinq_user_functions.h"

iinq_prepared_sql
insert_1(
	int		value_1,
	char	*value_2,
	char	*value_3,
	int		value_4,
	char	*value_5
) {
	iinq_prepared_sql p = { 0 };

	p.table				= malloc(sizeof(int));
	*(int *) p.table	= 1;
	p.value				= malloc((sizeof(int) * 2) + (sizeof(char) * 80));

	unsigned char *data = p.value;

	p.key			= malloc(sizeof(int));
	*(int *) p.key	= value_1;

	*(int *) data	= value_1;
	data			+= sizeof(int);

	if (value_2 != NULL) {
		memcpy(data, value_2, (sizeof(char) * 20));
	}
	else {
		*(char *) data = NULL;
	}

	data += (sizeof(char) * 20);

	if (value_3 != NULL) {
		memcpy(data, value_3, (sizeof(char) * 30));
	}
	else {
		*(char *) data = NULL;
	}

	data			+= (sizeof(char) * 30);

	*(int *) data	= value_4;
	data			+= sizeof(int);

	if (value_5 != NULL) {
		memcpy(data, value_5, (sizeof(char) * 30));
	}
	else {
		*(char *) data = NULL;
	}

	return p;
}

void
print_table_0(
	ion_dictionary_t *dictionary
) {
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(sizeof(int));
	ion_record.value	= malloc((sizeof(int) * 2) + (sizeof(char) * 80));

	printf("Table: dogs\n");
	printf("ID	");
	printf("TYPE	");
	printf("NAME	");
	printf("AGE	");
	printf("CITY	");
	printf("\n***************************************\n");

	ion_cursor_status_t cursor_status;
	unsigned char		*value;

	while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
		value	= ion_record.value;

		printf("%i	", NEUTRALIZE(value, int));
		value	+= sizeof(int);

		printf("%s	", (char *) value);
		value	+= (sizeof(char) * 20);

		printf("%s	", (char *) value);
		value	+= (sizeof(char) * 30);

		printf("%i	", NEUTRALIZE(value, int));
		value	+= sizeof(int);

		printf("%s	", (char *) value);

		printf("\n");
	}

	printf("\n");

	cursor->destroy(&cursor);
	free(ion_record.key);
	free(ion_record.value);
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

	if (value_2 != NULL) {
		memcpy(data, value_2, (sizeof(char) * 20));
	}
	else {
		*(char *) data = NULL;
	}

	data += (sizeof(char) * 20);

	if (value_3 != NULL) {
		memcpy(data, value_3, (sizeof(char) * 30));
	}
	else {
		*(char *) data = NULL;
	}

	data			+= (sizeof(char) * 30);

	*(int *) data	= value_4;
	data			+= sizeof(int);

	if (value_5 != NULL) {
		memcpy(data, value_5, (sizeof(char) * 30));
	}
	else {
		*(char *) data = NULL;
	}

	return p;
}

void
print_table_1(
	ion_dictionary_t *dictionary
) {
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(sizeof(int));
	ion_record.value	= malloc((sizeof(int) * 2) + (sizeof(char) * 80));

	printf("Table: dogs\n");
	printf("ID	");
	printf("TYPE	");
	printf("NAME	");
	printf("AGE	");
	printf("CITY	");
	printf("\n***************************************\n");

	ion_cursor_status_t cursor_status;
	unsigned char		*value;

	while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
		value	= ion_record.value;

		printf("%i	", NEUTRALIZE(value, int));
		value	+= sizeof(int);

		printf("%s	", (char *) value);
		value	+= (sizeof(char) * 20);

		printf("%s	", (char *) value);
		value	+= (sizeof(char) * 30);

		printf("%i	", NEUTRALIZE(value, int));
		value	+= sizeof(int);

		printf("%s	", (char *) value);

		printf("\n");
	}

	printf("\n");

	cursor->destroy(&cursor);
	free(ion_record.key);
	free(ion_record.value);
}

void
print_table_2(
	ion_dictionary_t *dictionary
) {
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc((sizeof(char) * 2));
	ion_record.value	= malloc((sizeof(int) * 1) + (sizeof(char) * 32));

	printf("Table: cats\n");
	printf("ID	");
	printf("NAME	");
	printf("AGE	");
	printf("\n***************************************\n");

	ion_cursor_status_t cursor_status;
	unsigned char		*value;

	while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
		value	= ion_record.value;

		printf("%s	", (char *) value);
		value	+= (sizeof(char) * 2);

		printf("%s	", (char *) value);
		value	+= (sizeof(char) * 30);

		printf("%i	", NEUTRALIZE(value, int));

		printf("\n");
	}

	printf("\n");

	cursor->destroy(&cursor);
	free(ion_record.key);
	free(ion_record.value);
}

iinq_prepared_sql
insert_2(
	char	*value_1,
	char	*value_2,
	int		value_3
) {
	iinq_prepared_sql p = { 0 };

	p.table				= malloc(sizeof(int));
	*(int *) p.table	= 2;
	p.value				= malloc((sizeof(int) * 1) + (sizeof(char) * 32));

	unsigned char *data = p.value;

	p.key = malloc((sizeof(char) * 2));
	memcpy(p.key, value_1, 12);

	if (value_1 != NULL) {
		memcpy(data, value_1, (sizeof(char) * 2));
	}
	else {
		*(char *) data = NULL;
	}

	data += (sizeof(char) * 2);

	if (value_2 != NULL) {
		memcpy(data, value_2, (sizeof(char) * 30));
	}
	else {
		*(char *) data = NULL;
	}

	data			+= (sizeof(char) * 30);

	*(int *) data	= value_3;

	return p;
}

void
setParam(
	iinq_prepared_sql	p,
	int					field_num,
	void				*val
) {
	unsigned char *data = p.value;

	iinq_field_t type	= getFieldType(p.table, field_num);

	data += calculateOffset(p.table, (field_num - 1));

	if (type == iinq_int) {
		*(int *) data = (int) val;
	}
	else {
		memcpy(data, val, sizeof(val));
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

		case 2: {
			switch (field_num) {
				case 1:
					return iinq_null_terminated_string;

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
	iinq_table_id		table_id,
	iinq_print_table_t	print_function,
	ion_key_type_t		key_type,
	size_t				key_size,
	size_t				value_size,
	int					num_wheres,
	int					num_update,
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

		if (!condition_satisfied || (num_wheres == 0)) {
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

				value = ion_record.value + calculateOffset(table_id, updates[i].implicit_field - 1);

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

				value			= ion_record.value + calculateOffset(table_id, updates[i].update_field - 1);
				*(int *) value	= new_value;
			}
			else {
				value = ion_record.value + calculateOffset(table_id, updates[i].update_field - 1);

				if (getFieldType(table_id, updates[i].update_field) == iinq_int) {
					*(int *) value = (int) updates[i].field_value;
				}
				else {
					memcpy(value, updates[i].field_value, calculateOffset(table_id, updates[i].update_field) - calculateOffset(table_id, updates[i - 1].update_field));
				}
			}
		}

		error = dictionary_update(&dictionary, ion_record.key, ion_record.value).error;

		if (err_ok != error) {
			printf("Error occurred. Error code: %i\n", error);
		}
	}

	cursor_temp->destroy(&cursor_temp);

	if (NULL != print_function) {
		print_function(&dictionary);
	}

	iinq_drop(255);
	free(ion_record.key);
	free(ion_record.value);
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
			iinq_execute(1, IONIZE(*(int *) p.key, int), p.value, iinq_insert_t);
			break;
		}

		case 2: {
			iinq_execute(2, p.key, p.value, iinq_insert_t);
			break;
		}
	}

	free(p.value);
	free(p.table);
	free(p.key);
}

void
delete_record(
	iinq_table_id		table_id,
	iinq_print_table_t	print_function,
	ion_key_type_t		key_type,
	size_t				key_size,
	size_t				value_size,
	int					num_fields,
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

		if (!condition_satisfied || (num_fields == 0)) {
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

	if (NULL != print_function) {
		print_function(&dictionary);
	}

	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	iinq_drop(255);
	free(ion_record.key);
	free(ion_record.value);
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
					return sizeof(int);

				case 2:
					return sizeof(int) + (sizeof(char) * 20);

				case 3:
					return sizeof(int) + (sizeof(char) * 20) + (sizeof(char) * 30);

				case 4:
					return sizeof(int) + (sizeof(char) * 20) + (sizeof(char) * 30) + sizeof(int);

				case 5:
					return sizeof(int) + (sizeof(char) * 20) + (sizeof(char) * 30) + sizeof(int) + (sizeof(char) * 30);

				default:
					return 0;
			}
		}

		case 1: {
			switch (field_num) {
				case 1:
					return sizeof(int);

				case 2:
					return sizeof(int) + (sizeof(char) * 20);

				case 3:
					return sizeof(int) + (sizeof(char) * 20) + (sizeof(char) * 30);

				case 4:
					return sizeof(int) + (sizeof(char) * 20) + (sizeof(char) * 30) + sizeof(int);

				case 5:
					return sizeof(int) + (sizeof(char) * 20) + (sizeof(char) * 30) + sizeof(int) + (sizeof(char) * 30);

				default:
					return 0;
			}
		}

		case 2: {
			switch (field_num) {
				case 1:
					return sizeof(char) * 2;

				case 2:
					return (sizeof(char) * 2) + (sizeof(char) * 30);

				case 3:
					return (sizeof(char) * 2) + (sizeof(char) * 30) + sizeof(int);

				default:
					return 0;
			}
		}

		default:
			return 0;
	}
}
