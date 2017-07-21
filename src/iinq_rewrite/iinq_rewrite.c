#include "iinq_rewrite.h"
#include "../iinq/iinq.h"
#include "../util/sort/external_sort/external_sort_types.h"

ion_err_t
iinq_create_table(
	char				*schema_file_name,
	ion_key_type_t		key_type,
	ion_key_size_t		key_size,
	ion_value_size_t	value_size,
	iinq_schema_t		*schema
) {
	ion_err_t					error;
	FILE						*schema_file;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= ion_init_master_table();

	if (err_ok != error) {
		return error;
	}

	/* Load the handler. */
	ffdict_init(&handler);

	/* If the file exists, fail. */
	if (NULL != (schema_file = fopen(schema_file_name, "rb"))) {
		if (0 != fclose(schema_file)) {
			return err_file_close_error;
		}

		return err_duplicate_dictionary_error;
	}
	/* Otherwise, we are creating the dictionary for the first time. */
	else if (NULL != (schema_file = fopen(schema_file_name, "w+b"))) {
		if (0 != fseek(schema_file, 0, SEEK_SET)) {
			return err_file_bad_seek;
		}

		error = ion_master_table_create_dictionary(&handler, &dictionary, key_type, key_size, value_size, 10);

		if (err_ok != error) {
			return error;
		}

		if (1 != fwrite(&dictionary.instance->id, sizeof(dictionary.instance->id), 1, schema_file)) {
			return err_file_incomplete_write;
		}

		/* write number of fields in table */
		if (1 != fwrite(&schema->num_fields, sizeof(iinq_num_fields_t), 1, schema_file)) {
			return err_file_incomplete_write;
		}

		/* write field types in table */
		if (schema->num_fields != fwrite(schema->field_type, sizeof(iinq_field_type_t), schema->num_fields, schema_file)) {
			return err_file_incomplete_write;
		}

		/* write field sizes in table */
		if (schema->num_fields != fwrite(schema->field_size, sizeof(iinq_field_size_t), schema->num_fields, schema_file)) {
			return err_file_incomplete_write;
		}

		if (0 != fclose(schema_file)) {
			return err_file_close_error;
		}

		ion_close_dictionary(&dictionary);

		error = err_ok;
	}
	else {
		error = err_file_open_error;
	}

	ion_close_master_table();

	return error;
}

ion_boolean_t
iinq_string_comparison(
	const char					*value,
	iinq_comparison_operator_t	operator,
	const char					*comp_value
) {
	switch (operator) {
		case IINQ_EQUAL_TO:
			return strcmp(value, comp_value) == 0 ? boolean_true : boolean_false;
			break;

		case IINQ_NOT_EQUAL_TO:
			return strcmp(value, comp_value) != 0 ? boolean_true : boolean_false;
			break;

		case IINQ_GREATER_THAN:
			return strcmp(value, comp_value) > 0 ? boolean_true : boolean_false;
			break;

		case IINQ_GREATER_EQUAL_TO:
			return strcmp(value, comp_value) >= 0 ? boolean_true : boolean_false;
			break;

		case IINQ_LESS_THAN:
			return strcmp(value, comp_value) < 0 ? boolean_true : boolean_false;
			break;

		case IINQ_LESS_THAN_EQUAL_TO:
			return strcmp(value, comp_value) <= 0 ? boolean_true : boolean_false;
			break;

		default:
			return boolean_false;
	}
}

ion_boolean_t
iinq_int_comparison(
	int							value,
	iinq_comparison_operator_t	operator,
	int							comp_value
) {
	switch (operator) {
		case IINQ_EQUAL_TO:
			return (value == comp_value) ? boolean_true : boolean_false;
			break;

		case IINQ_NOT_EQUAL_TO:
			return (value != comp_value) ? boolean_true : boolean_false;
			break;

		case IINQ_GREATER_THAN:
			return (value > comp_value) ? boolean_true : boolean_false;
			break;

		case IINQ_GREATER_EQUAL_TO:
			return (value >= comp_value) ? boolean_true : boolean_false;
			break;

		case IINQ_LESS_THAN:
			return (value < comp_value) ? boolean_true : boolean_false;
			break;

		case IINQ_LESS_THAN_EQUAL_TO:
			return (value <= comp_value) ? boolean_true : boolean_false;
			break;

		default:
			return boolean_false;
	}
}

ion_boolean_t
iinq_double_comparison(
	double						value,
	iinq_comparison_operator_t	operator,
	int							comp_value
) {
	/* TODO: Float inaccuracy causes errors (e.g 2.499999999 < 2.5 == boolean_true when it should be false) */
	switch (operator) {
		case IINQ_EQUAL_TO:
			return (abs(value - comp_value) < FLOAT_TOLERANCE) ? boolean_true : boolean_false;
			break;

		case IINQ_NOT_EQUAL_TO:
			return (abs(value - comp_value) > FLOAT_TOLERANCE) ? boolean_true : boolean_false;
			break;

		case IINQ_GREATER_THAN:
			return (value > comp_value) ? boolean_true : boolean_false;
			break;

		case IINQ_GREATER_EQUAL_TO:
			return (value > comp_value || abs(value - comp_value) < FLOAT_TOLERANCE) ? boolean_true : boolean_false;
			break;

		case IINQ_LESS_THAN:
			return (value < comp_value) ? boolean_true : boolean_false;
			break;

		case IINQ_LESS_THAN_EQUAL_TO:
			return (value < comp_value || abs(value - comp_value) < FLOAT_TOLERANCE) ? boolean_true : boolean_false;
			break;

		default:
			return boolean_false;
	}
}

ion_err_t
iinq_open_table(
	char			*name,
	iinq_table_t	*table
) {
	table->dictionary			= malloc(sizeof(ion_dictionary_t));

	table->dictionary->handler	= malloc(sizeof(ion_dictionary_handler_t));

	if (NULL == table->dictionary->handler) {
		free(table->dictionary);
		return err_out_of_memory;
	}

	/* same code as iinq_open_source(name, table->dictionary, table->dictionary->handler) but more stuff added in */

	ion_err_t			error;
	FILE				*schema_file;
	ion_dictionary_id_t id;

	error = ion_init_master_table();

	if (err_ok != error) {
		free(table->dictionary->handler);
		free(table->dictionary);
		return error;
	}

	/* Load the handler. */
	ffdict_init(table->dictionary->handler);

	/* If the schema file already exists. */
	if (NULL != (schema_file = fopen(name, "rb"))) {
		if (0 != fseek(schema_file, 0, SEEK_SET)) {
			free(table->dictionary->handler);
			free(table->dictionary);
			return err_file_bad_seek;
		}

		if (1 != fread(&id, sizeof(id), 1, schema_file)) {
			return err_file_incomplete_read;
		}

		error = ion_open_dictionary(table->dictionary->handler, table->dictionary, id);

		if (err_ok != error) {
			free(table->dictionary->handler);
			free(table->dictionary);
			return error;
		}

		table->schema = malloc(sizeof(iinq_schema_t));

		if (NULL == table->schema) {
			free(table->dictionary->handler);
			free(table->dictionary);
			return err_out_of_memory;
		}

		if (1 != fread(&table->schema->num_fields, sizeof(iinq_num_fields_t), 1, schema_file)) {
			free(table->schema);
			free(table->dictionary->handler);
			free(table->dictionary);
			return err_file_incomplete_read;
		}

		table->schema->field_type = malloc(sizeof(iinq_field_type_t) * table->schema->num_fields);

		if (NULL == table->schema->field_type) {
			free(table->schema);
			free(table->dictionary->handler);
			free(table->dictionary);
			return err_out_of_memory;
		}

		if (table->schema->num_fields != fread(table->schema->field_type, sizeof(iinq_field_type_t), table->schema->num_fields, schema_file)) {
			free(table->schema->num_fields);
			free(table->schema);
			free(table->dictionary->handler);
			free(table->dictionary);
			return err_file_incomplete_read;
		}

		table->schema->field_size = malloc(sizeof(iinq_field_size_t) * table->schema->num_fields);

		if (table->schema->num_fields != fread(table->schema->field_size, sizeof(iinq_field_size_t), table->schema->num_fields, schema_file)) {
			free(table->schema->num_fields);
			free(table->schema);
			free(table->dictionary->handler);
			free(table->dictionary);
			return err_file_incomplete_read;
		}

		if (0 != fclose(schema_file)) {
			return err_file_close_error;
		}

		error = err_ok;
	}
	else {
		error = err_file_open_error;
	}

	ion_close_master_table();

	if (err_ok != error) {
		free(table->dictionary->handler);
		free(table->dictionary);
		return error;
	}

	/* build the predicate (predicate_all_records unless we are filtering by key) */
	ion_predicate_t predicate;

	error = dictionary_build_predicate(&predicate, predicate_all_records);

	if (err_ok != error) {
		free(table->dictionary->handler);
		free(table->dictionary);
		return error;
	}

	error = dictionary_find(table->dictionary, &predicate, &table->cursor);

	if (err_ok != error) {
		free(table->dictionary->handler);
		free(table->dictionary);
	}

	return error;
}

void
iinq_init_where(
	iinq_iterator_t *it,
	int				num_filters,
	va_list			*filters
) {
	it->query->filter = malloc(sizeof(iinq_where_filter_t) * num_filters);

	iinq_where_filter_t *filter;

	filter = va_arg(*filters, iinq_where_filter_t *);

	for (int i = 0; i < num_filters; i++) {
		it->query->filter[i].operator	= filter->operator;
		it->query->filter[i].field_num	= filter->field_num;
		it->query->filter[i].comp_value = malloc(it->query->tables[0].schema->field_size[filter->field_num]);
		memcpy(it->query->filter[i].comp_value, filter->comp_value, it->query->tables[0].schema->field_size[filter->field_num]);
		filter++;
	}
}

iinq_field_t
iinq_get_field_from_table(
	iinq_iterator_t *it,
	int				table_num,
	int				field_num
) {
	iinq_field_t field;

	if (0 == field_num) {
		/* Key was selected */
		field = it->query->tables[table_num].record.key;
	}
	else {
		/* A field within value was selected */
		field = it->query->tables[table_num].record.value;

		for (int i = 1; i < field_num; i++) {
			field += it->query->tables->schema->field_size[i];
		}
	}

	return field;
}

iinq_field_t
iinq_get_field_from_sort(
	iinq_iterator_t *it,
	int				sort_num,
	int				field_num
) {
	iinq_field_t field;

	field = it->query->sort->record_buf + it->query->sort->size;

	for (int i = 0; i < field_num; i++) {
		field += it->query->tables->schema->field_size[i];
	}

	return field;
}

ion_boolean_t
iinq_where_from_table(
	iinq_iterator_t *it
) {
	if (NULL == it->query->filter) {
		return boolean_true;
	}

	iinq_where_filter_t *filter;

	for (int i = 0; i < it->query->num_filters; i++) {
		filter = &it->query->filter[i];

		int field_num = filter->field_num;

		switch (it->query->tables[0].schema->field_type[field_num]) {
			case IINQ_STRING: {
				char *value = (char *) iinq_get_field_from_table(it, 0, filter->field_num);

				if (!iinq_string_comparison(value, filter->operator, (char *) filter->comp_value)) {
					return boolean_false;
				}

				break;
			}

			case IINQ_INT: {
				int value = *(int *) iinq_get_field_from_table(it, 0, filter->field_num);

				if (!iinq_int_comparison(value, filter->operator, *(int *) filter->comp_value)) {
					return boolean_false;
				}

				break;
			}

			case IINQ_DOUBLE: {
				double value = *(double *) iinq_get_field_from_table(it, 0, filter->field_num);

				if (iinq_double_comparison(value, filter->operator, *(double *) filter->comp_value)) {
					return boolean_false;
				}

				break;
			}
		}
	}
}

iinq_iterator_status_t
select_all_next(
	iinq_iterator_t *it
) {
	ion_boolean_t valid = boolean_false;

	while (!valid) {
		if (cs_cursor_active != it->query->tables->cursor->next(it->query->tables->cursor, &it->query->tables->record)) {
			return it->status = it_status_end_of_results;
		}
		else if (!iinq_where_from_table(it)) {
			continue;
		}
		else {
			valid = boolean_true;
		}
	}

	for (int i = 0; i < it->query->tables[0].schema->num_fields; i++) {
		it->query->tuple.fields[i] = iinq_get_field_from_table(it, 0, i);
	}

	return it->status;
}

iinq_iterator_status_t
select_field_list_next(
	iinq_iterator_t *it
) {
	ion_boolean_t valid = boolean_false;

	while (!valid) {
		if (cs_cursor_active != it->query->tables->cursor->next(it->query->tables->cursor, &it->query->tables->record)) {
			return it->status = it_status_end_of_results;
		}

		if (!iinq_where_from_table(it)) {
			continue;
		}
		else {
			valid = boolean_true;
		}
	}

	iinq_field_list_t *field_list = it->query->tuple.field_list;

	for (int i = 0; i < it->query->tuple.schema->num_fields; i++) {
		int table_num	= field_list[i].table_num;
		int field_num	= field_list[i].field_num;

		it->query->tuple.fields[i] = iinq_get_field_from_table(it, table_num, field_num);
	}

	return it->status;
}

iinq_iterator_status_t
select_all_order_by_next(
	iinq_iterator_t *it
) {
	ion_err_t error = it->query->sort->cursor->next(it->query->sort->cursor, it->query->sort->record_buf);

	if (err_ok != error) {
		return it->status = it_status_sort_error;
	}

	char *sort_data = it->query->sort->record_buf;

	for (int i = 0; i < it->query->tuple.schema->num_fields; i++) {
		it->query->tuple.fields[i]	= sort_data;
		sort_data					+= it->query->tables[0].schema->field_size[i];
	}

	return it->status;
}

iinq_iterator_status_t
select_field_list_order_by_next(
	iinq_iterator_t *it
) {
	ion_err_t error = it->query->sort->cursor->next(it->query->sort->cursor, it->query->sort->record_buf);

	if (err_ok != error) {
		return it->status = it_status_sort_error;
	}

	if (cs_cursor_active != it->query->sort->cursor->status) {
		return it->status = it_status_end_of_results;
	}

	iinq_field_list_t *field_list = it->query->tuple.field_list;

	for (int i = 0; i < it->query->tuple.schema->num_fields; i++) {
		int table_num	= field_list[i].table_num;
		int field_num	= field_list[i].field_num;

		it->query->tuple.fields[i] = iinq_get_field_from_sort(it, table_num, field_num);
	}

	return it->status;
}

iinq_iterator_status_t
init(
	iinq_iterator_t			*it,
	iinq_select_type_t		select_type,
	iinq_order_by_type_t	order_by_type,
	int						num_tables,
	int						num_filters,
	...
) {
	int		i;
	va_list list;

	va_start(list, num_filters);

	it->query			= malloc(sizeof(iinq_query_t));
	/* multiply sizeof(ion_iinq_table_t) by number of dictionaries used in query */
	it->query->tables	= malloc(sizeof(iinq_table_t) * num_tables);

	if (NULL == it->query->tables) {
		return it->status = it_status_invalid;
	}

	ion_err_t error;

	for (i = 0; i < num_tables; i++) {
		char name[ION_MAX_FILENAME_LENGTH];

		strcpy(name, va_arg(list, char *));
		it->query->tables[i].schema = malloc(sizeof(iinq_schema_t));
		error						= iinq_open_table(strcat(name, ".inq"), &it->query->tables[i]);

		/* predicate used to look through table,
		 * for each table with multiple tables */
		if (err_ok != error) {
			return it->status = it_status_invalid;
		}

		it->query->tables->record.key = malloc(it->query->tables->dictionary->instance->record.key_size);

		if (NULL == it->query->tables->record.key) {
			ion_close_dictionary(it->query->tables);
			free(it->query->tables);
			it->status = it_status_invalid;
			return it;
		}

		it->query->tables->record.value = malloc(it->query->tables->dictionary->instance->record.value_size);

		if (NULL == it->query->tables->record.value) {
			free(it->query->tables->record.key);
			ion_close_dictionary(it->query->tables);
			free(it->query->tables);
			it->status = it_status_invalid;
			return it;
		}
	}

	it->query->num_filters = num_filters;

	if (0 != num_filters) {
		/* initialize the WHERE based on the filters passed */
		iinq_init_where(it, num_filters, &list);
	}
	else {
		it->query->filter = NULL;
	}

	it->query->select_type	= select_type;
	it->query->tuple.schema = malloc(sizeof(iinq_schema_t));

	/* if SELECT field_list */
	if (IINQ_SELECT_FIELD_LIST == select_type) {
		/* get the number of fields to be selected */
		it->query->tuple.schema->num_fields = va_arg(list, int);
		/* allocate the appropriate number of pointers for the data */
		it->query->tuple.fields				= malloc(sizeof(iinq_field_t) * it->query->tuple.schema->num_fields);

		if (IINQ_ORDER_BY_NONE == order_by_type) {
			it->next = select_field_list_next;
		}
		else if (IINQ_ORDER_BY_FIELD == order_by_type) {
			it->next = select_field_list_order_by_next;
		}

		if (NULL == it->query->tuple.fields) {
			ion_close_dictionary(it->query->tables);
			free(it->query->tables);
			free(it->query->tables->record.key);
			free(it->query->tables->record.value);
			it->status = it_status_invalid;
			return it;
		}

		iinq_field_list_t *field_list;

		it->query->tuple.field_list = malloc(sizeof(iinq_field_list_t) * it->query->tuple.schema->num_fields);
		it->query->tuple.size		= 0;
		field_list					= va_arg(list, iinq_field_list_t *);

		for (i = 0; i < it->query->tuple.schema->num_fields; i++) {
			it->query->tuple.field_list[i].table_num	= field_list->table_num;
			it->query->tuple.field_list[i].field_num	= field_list->field_num;
			it->query->tuple.size						+= it->query->tables[field_list->table_num].schema->field_size[field_list->field_num];
			field_list++;
		}
	}
	else {
		/* else SELECT * */
		it->query->tuple.is_select_all		= boolean_true;
		it->query->tuple.field_list			= NULL;
		it->query->tuple.size				= 0;
		it->query->tuple.schema->num_fields = 0;
		it->query->tuple.size				+= it->query->tables[0].dictionary->instance->record.key_size;
		it->query->tuple.size				+= it->query->tables[0].dictionary->instance->record.value_size;
		it->query->tuple.schema->num_fields += it->query->tables[0].schema->num_fields;
		it->query->tuple.schema->field_type = malloc(sizeof(iinq_field_type_t) * it->query->tables[0].schema->num_fields);
		it->query->tuple.schema->field_size = malloc(sizeof(iinq_field_size_t) * it->query->tables[0].schema->num_fields);

		for (i = 0; i < it->query->tables[0].schema->num_fields; i++) {
			it->query->tuple.schema->field_type[i]	= it->query->tables[0].schema->field_type[i];
			it->query->tuple.schema->field_size[i]	= it->query->tables[0].schema->field_size[i];
		}

		it->query->tuple.fields = malloc(sizeof(iinq_field_t) * it->query->tuple.schema->num_fields);

		if (IINQ_ORDER_BY_NONE == order_by_type) {
			it->next = select_all_next;
		}
		else if (IINQ_ORDER_BY_FIELD == order_by_type) {
			it->next = select_all_order_by_next;
		}
	}

	/* if ORDER BY attribute */
	if (IINQ_ORDER_BY_NONE != order_by_type) {
		if (IINQ_ORDER_BY_FIELD == order_by_type) {
			/* Only single order by is supported, hardcoded for now */
			int						orderby_n				= 1;
			iinq_order_by_field_t	*order_by_field			= va_arg(list, iinq_order_by_field_t *);
			int						total_orderby_size		= 0;
			iinq_order_part_t		*orderby_order_parts	= malloc(sizeof(iinq_order_part_t));

			if (0 == order_by_field->field_info.field_num) {
				orderby_order_parts[0].pointer = it->query->tables[order_by_field->field_info.table_num].record.key;
			}
			else {
				orderby_order_parts[0].pointer = iinq_get_field_from_table(it, order_by_field->field_info.table_num, order_by_field->field_info.field_num);
			}

			orderby_order_parts[0].direction	= order_by_field->direction;
			orderby_order_parts[0].size			= it->query->tables[order_by_field->field_info.table_num].schema->field_size[order_by_field->field_info.field_num];

			switch (it->query->tables[order_by_field->field_info.table_num].schema->field_type[order_by_field->field_info.field_num]) {
				case IINQ_INT:
					orderby_order_parts[0].type = IINQ_ORDERTYPE_INT;
					break;

				case IINQ_UINT:
					orderby_order_parts[0].type = IINQ_ORDERTYPE_UINT;
					break;

				case IINQ_DOUBLE:
					orderby_order_parts[0].type = IINQ_ORDERTYPE_FLOAT;
					break;

				case IINQ_STRING:
					orderby_order_parts[0].type = IINQ_ORDERTYPE_OTHER;
					break;
			}

			total_orderby_size += orderby_order_parts[0].size;

			FILE *file = fopen("orderby", "wb");

			if (NULL == file) {
				ion_close_dictionary(it->query->tables);
				free(it->query->tables);
				free(it->query->tables->record.key);
				free(it->query->tables->record.value);
				free(it->query->tuple.fields);
				it->status = it_status_invalid;
				return it;
			}

			int					write_page_remaining	= IINQ_PAGE_SIZE;
			ion_key_size_t		key_size				= it->query->tables[0].dictionary->instance->record.key_size;
			ion_value_size_t	value_size				= it->query->tables[0].dictionary->instance->record.value_size;

			/* filter before sorting,
			 * cursor already initialized via iinq_open_table */
			while (cs_cursor_active == it->query->tables->cursor->next(it->query->tables->cursor, &it->query->tables->record)) {
				if (!iinq_where_from_table(it)) {
					continue;
				}

				if (write_page_remaining < (total_orderby_size + key_size + value_size)) {
					char x = 0;

					for (i = 0; i < write_page_remaining; i++) {
						if (1 != fwrite(&x, 1, 1, file)) {
							break;
						}
					}

					write_page_remaining = IINQ_PAGE_SIZE;
				}

				for (i = 0; i < orderby_n; i++) {
					if (1 != fwrite(orderby_order_parts[i].pointer, orderby_order_parts[i].size, 1, file)) {
						break;
					}
					else {
						write_page_remaining -= orderby_order_parts[i].size;
					}
				}

				if (1 != fwrite(it->query->tables->record.key, key_size, 1, file)) {
					break;
				}
				else {
					write_page_remaining -= key_size;
				}

				if (1 != fwrite(it->query->tables->record.value, value_size, 1, file)) {
					break;
				}
				else {
					write_page_remaining -= value_size;
				}
			}

			if (NULL != file) {
				fclose(file);
			}

			/* open file in read mode for sorting */
			file = fopen("orderby", "rb");

			iinq_sort_context_t *context = malloc(sizeof(iinq_sort_context_t));

			*context = _IINQ_SORT_CONTEXT(orderby);

			ion_external_sort_t *es = malloc(sizeof(ion_external_sort_t));

			error = ion_external_sort_init(es, file, context, iinq_sort_compare, key_size + value_size, key_size + value_size + total_orderby_size, IINQ_PAGE_SIZE, boolean_false, ION_FILE_SORT_FLASH_MINSORT);

			uint16_t buffer_size = ion_external_sort_bytes_of_memory_required(es, 0, boolean_false);

			it->query->sort			= malloc(sizeof(iinq_sort_t));
			it->query->sort->size	= total_orderby_size;

			char *buffer = malloc(buffer_size);

			/* recordbuf needs enough room for the sort field and the table tuple (sort field is stored twice)
			 * projection is done afterward */
			it->query->sort->record_buf = malloc((total_orderby_size + key_size + value_size));

			it->query->sort->cursor		= malloc(sizeof(ion_external_sort_cursor_t));

			if (err_ok != (error = ion_external_sort_init_cursor(es, it->query->sort->cursor, buffer, buffer_size))) {
				free(it->query->tables);
				free(it->query->tables->record.key);
				free(it->query->tables->record.value);
				free(it->query->tuple.fields);
				it->status = it_status_invalid;
				fclose(file);
				return it;
			}

			if (NULL == file) {
				ion_close_dictionary(it->query->tables);
				free(it->query->tables);
				free(it->query->tables->record.key);
				free(it->query->tables->record.value);
				free(it->query->tuple.fields);
				it->status = it_status_invalid;
				return it;
			}
		}
	}

	it->status = it_status_ok;

	va_end(num_filters);

	return it;
}
