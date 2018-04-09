#include "iinq_user_functions.h"

void
print_table_cats(
	ion_dictionary_t *dictionary
) {
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(sizeof(int));
	ion_record.value	= malloc((sizeof(int) * 2) + (sizeof(char) * 30));

	printf("Table: Cats\n");
	printf("id	");
	printf("name	");
	printf("age	");
	printf("\n***************************************\n");

	ion_cursor_status_t cursor_status;
	unsigned char		*value;

	while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
		value	= ion_record.value;

		printf("%i	", NEUTRALIZE(value, int));
		value	+= sizeof(int);

		printf("%s	", (char *) value);
		value	+= sizeof(char) * 30;

		printf("%i	", NEUTRALIZE(value, int));

		printf("\n");
	}

	printf("\n");

	cursor->destroy(&cursor);
	free(ion_record.key);
	free(ion_record.value);
}

void
create_table(
	char				*table_name,
	ion_key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size
) {
	ion_err_t error = iinq_create_source(table_name, key_type, key_size, value_size);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}
}

iinq_prepared_sql
insert_Cats(
	int		value_1,
	char	*value_2,
	int		value_3
) {
	iinq_prepared_sql p = { 0 };

	p.table				= malloc(sizeof(int));
	*(int *) p.table	= 0;
	p.value				= malloc((sizeof(int) * 2) + (sizeof(char) * 30));

	unsigned char *data = p.value;

	p.key			= malloc(sizeof(int));
	*(int *) p.key	= value_1;

	*(int *) data	= value_1;
	data			+= sizeof(int);

	memcpy(data, value_2, sizeof(char) * 30);
	data			+= sizeof(char) * 30;

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

void
execute(
	iinq_prepared_sql p
) {
	if (*(int *) p.table == 0) {
		iinq_execute("Cats.inq", IONIZE(*(int *) p.key, int), p.value, iinq_insert_t);
	}

	free(p.value);
	free(p.table);
	free(p.key);
}

void
delete_record(
	int				id,
	char			*name,
	ion_key_type_t	key_type,
	size_t			key_size,
	size_t			value_size,
	int				num_fields,
	...
) {
	va_list valist;

	va_start(valist, num_fields);

	unsigned char *table_id = malloc(sizeof(int));

	*(int *) table_id = id;

	char *table_name = malloc(sizeof(char) * 20);

	memcpy(table_name, name, sizeof(char) * 20);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table_name, &dictionary, &handler);

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

	error = iinq_create_source("DEL.inq", key_type, (ion_key_size_t) key_size, (ion_value_size_t) sizeof(int));

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	ion_dictionary_t			dictionary_temp;
	ion_dictionary_handler_t	handler_temp;

	dictionary_temp.handler = &handler_temp;

	error					= iinq_open_source("DEL.inq", &dictionary_temp, &handler_temp);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	ion_boolean_t condition_satisfied;

	while ((status = iinq_next_record(cursor, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		condition_satisfied = where(table_id, &ion_record, num_fields, &valist);

		if (!condition_satisfied) {
			error = dictionary_insert(&dictionary_temp, ion_record.key, IONIZE(0, int)).error;

			if (err_ok != error) {
				printf("Error occurred. Error code: %i\n", error);
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
		}
	}

	cursor_temp->destroy(&cursor_temp);
/*	print_table_cats(&dictionary); */

	error = iinq_drop("DEL.inq");

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	free(table_id);
	free(table_name);
	free(ion_record.key);
	free(ion_record.value);
}

void
update(
	int				id,
	char			*name,
	ion_key_type_t	key_type,
	size_t			key_size,
	size_t			value_size,
	int				num_wheres,
	int				num_update,
	int				num,
	...
) {
	va_list valist;

	va_start(valist, num);

	unsigned char *table_id = malloc(sizeof(int));

	*(int *) table_id = id;

	char *table_name = malloc(sizeof(char) * 20);

	memcpy(table_name, name, sizeof(char) * 20);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table_name, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error occurred 221. Error code: %i, out of mem: %i\n", error, err_out_of_memory);
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

	error = iinq_create_source("UPD.inq", key_type, (ion_key_size_t) key_size, (ion_value_size_t) value_size);

	if (err_ok != error) {
		printf("Error occurred 240. Error code: %i, out of mem: %i\n", error, err_out_of_memory);
		return;
	}

	ion_dictionary_t			dictionary_temp;
	ion_dictionary_handler_t	handler_temp;

	dictionary_temp.handler = &handler_temp;

	error					= iinq_open_source("UPD.inq", &dictionary_temp, &handler_temp);

	if (err_ok != error) {
		printf("Error occurred 252. Error code: %i, out of mem: %i\n", error, err_out_of_memory);
		return;
	}

	ion_boolean_t condition_satisfied;

	while ((status = iinq_next_record(cursor, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		condition_satisfied = where(table_id, &ion_record, num_wheres, &valist);

		if (!condition_satisfied || (num_wheres == 0)) {
			error = dictionary_insert(&dictionary_temp, ion_record.key, ion_record.value).error;

			if (err_ok != error) {
				printf("Error occurred 265. Error code: %i, out of mem: %i\n", error, err_out_of_memory);
			}
		}
	}

	cursor->destroy(&cursor);

	int						update_fields[num_update / 4];
	int						implicit_fields[num_update / 4];
	iinq_math_operator_t	operators[num_update / 4];
	void					*field_values[num_update / 4];
	int						i;

	for (i = 0; i < num_wheres; i++) {
		va_arg(valist, void *);
	}

	for (i = 0; i < num_update / 4; i++) {
		update_fields[i]	= va_arg(valist, int);
		implicit_fields[i]	= va_arg(valist, int);
		operators[i]		= va_arg(valist, iinq_math_operator_t);
		field_values[i]		= va_arg(valist, void *);
	}

	va_end(valist);

	ion_predicate_t predicate_temp;

	dictionary_build_predicate(&predicate_temp, predicate_all_records);

	ion_dict_cursor_t *cursor_temp = NULL;

	dictionary_find(&dictionary_temp, &predicate_temp, &cursor_temp);

	while ((status = iinq_next_record(cursor_temp, &ion_record)) == cs_cursor_initialized || status == cs_cursor_active) {
		for (i = 0; i < num_update / 4; i++) {
			unsigned char *value;

			if (implicit_fields[i] != 0) {
				int new_value;

				value = ion_record.value + calculateOffset(table_id, implicit_fields[i] - 1);

				switch (operators[i]) {
					case iinq_add:
						new_value = (NEUTRALIZE(value, int) + (int) field_values[i]);
						break;

					case iinq_subtract:
						new_value = (NEUTRALIZE(value, int) - (int) field_values[i]);
						break;

					case iinq_multiply:
						new_value = (NEUTRALIZE(value, int) * (int) field_values[i]);
						break;

					case iinq_divide:
						new_value = (NEUTRALIZE(value, int) / (int) field_values[i]);
						break;
				}

				value			= ion_record.value + calculateOffset(table_id, update_fields[i] - 1);
				*(int *) value	= new_value;
			}
			else {
				value = ion_record.value + calculateOffset(table_id, update_fields[i] - 1);

				if (getFieldType(table_id, update_fields[i]) == iinq_int) {
					*(int *) value = (int) field_values[i];
				}
				else {
					memcpy(value, field_values[i], calculateOffset(table_id, update_fields[i]) - calculateOffset(table_id, update_fields[i - 1]));
				}
			}
		}

		error = dictionary_update(&dictionary, ion_record.key, ion_record.value).error;

		if (err_ok != error) {
			printf("Error occurred 336. Error code: %i, out of mem: %i\n", error, err_out_of_memory);
		}
	}

	cursor_temp->destroy(&cursor_temp);
/*	print_table_cats(&dictionary); */

	error = iinq_drop("UPD.inq");

	if (err_ok != error) {
		printf("Error occurred 346. Error code: %i, out of mem: %i\n", error, err_out_of_memory);
	}

	free(table_id);
	free(table_name);
	free(ion_record.key);
	free(ion_record.value);
}

iinq_result_set
iinq_select(
	int				id,
	char			*name,
	ion_key_type_t	key_type,
	size_t			key_size,
	size_t			value_size,
	int				num_wheres,
	int				num_fields,
	int				num,
	...
) {
	int		i;
	va_list valist, where_list;

	va_start(valist, num);
	va_copy(where_list, valist);

	unsigned char *table_id = malloc(sizeof(int));

	*(int *) table_id = id;

	char *table_name = malloc(sizeof(char) * 20);

	memcpy(table_name, name, sizeof(char) * 20);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table_name, &dictionary, &handler);

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

	int				count = 0;
	ion_boolean_t	condition_satisfied;

	int fields[num_fields];

	for (i = 0; i < num_wheres; i++) {
		va_arg(valist, void *);
	}

	iinq_result_set select = { 0 };

	select.num_fields			= malloc(sizeof(int));
	*(int *) select.num_fields	= num_fields;
	select.fields				= malloc(sizeof(int) * num_fields);

	unsigned char *field_list = select.fields;

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

	ffdict_init(&handler_temp);

	error = dictionary_create(&handler_temp, &dictionary_temp, 6, key_type, (ion_key_size_t) key_size, (ion_value_size_t) value_size, 10);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
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

			error = dictionary_insert(&dictionary_temp, IONIZE(count, int), fieldlist).error;

			if (err_ok != error) {
				printf("Error occurred. Error code: %i\n", error);
			}

			count++;
			free(fieldlist);
		}
	}

	cursor->destroy(&cursor);

	ion_predicate_t predicate_temp;

	dictionary_build_predicate(&predicate_temp, predicate_all_records);

	ion_dict_cursor_t *cursor_temp = NULL;

	select.record				= ion_record;
	select.table_id				= malloc(sizeof(int));
	*(int *) select.table_id	= id;
	select.cursor				= malloc(sizeof(ion_dict_cursor_t));

	dictionary_find(&dictionary_temp, &predicate_temp, &cursor_temp);
	select.cursor				= cursor_temp;

	free(table_id);
	free(table_name);

	return select;
}

ion_boolean_t
next(
	iinq_result_set *select
) {
	ion_cursor_status_t status = select->cursor->next(select->cursor, &select->record);

	if (status == cs_end_of_results) {
		free(select->cursor->dictionary->instance);
		free(select->cursor);
		free(select->record.key);
		free(select->table_id);
	}

	return status == cs_cursor_initialized || status == cs_cursor_active;
}

char *
getString(
	iinq_result_set *select,
	int				field_num
) {
	int i, count = 0;

	for (i = 0; i < *(int *) select->num_fields; i++) {
		int field = *(int *) (select->fields + sizeof(int) * i);

		if (getFieldType(select->table_id, field) == iinq_char) {
			count++;
		}

		if (count == field_num) {
			return (char *) (select->record.value + calculateOffset(select->table_id, field - 1));
		}
	}

	return "";
}

int
getInt(
	iinq_result_set *select,
	int				field_num
) {
	int i, count = 0;

	for (i = 0; i < *(int *) select->num_fields; i++) {
		int field = *(int *) (select->fields + sizeof(int) * i);

		if (getFieldType(select->table_id, field) == iinq_int) {
			count++;
		}

		if (count == field_num) {
			return NEUTRALIZE(select->record.value + calculateOffset(select->table_id, field - 1), int);
		}
	}

	return 0;
}

void
drop_table(
	char *table_name
) {
	ion_err_t error;

	error = iinq_drop(table_name);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	printf("Table %s has been deleted.\n", table_name);
}

size_t
calculateOffset(
	const unsigned char *table,
	int					field_num
) {
	switch (*(int *) table) {
		case 0: {
			switch (field_num) {
				case 1:
					return sizeof(int);

				case 2:
					return sizeof(int) + (sizeof(char) * 30);

				case 3:
					return (sizeof(int) * 2) + (sizeof(char) * 30);

				default:
					return 0;
			}
		}

		default:
			return 0;
	}
}

iinq_field_t
getFieldType(
	const unsigned char *table,
	int					field_num
) {
	switch (*(int *) table) {
		case 0: {
			switch (field_num) {
				case 1:
					return iinq_int;

				case 2:
					return iinq_char;

				case 3:
					return iinq_int;

				default:
					return iinq_int;
			}
		}

		default:
			return iinq_int;
	}
}
