#include "iinq_rewrite.h"
#include "../iinq/iinq.h"
#include "../util/sort/external_sort/external_sort_types.h"

void
iinq_destroy_tuple(iinq_tuple_t *tuple) {
	if (NULL != tuple->fields)
		free(tuple->fields);
	if (NULL != tuple->schema) {
		if (NULL != tuple->schema->field_type)
			free(tuple->schema->field_type);
		if (NULL != tuple->schema->field_size)
			free(tuple->schema->field_size);
		free(tuple->schema);
	}
	if (NULL != tuple->field_list)
		free(tuple->field_list);
}

void
iinq_destroy_query_tables(iinq_table_t **tables, int num_tables) {

	for (int i = 0; i < num_tables; i++) {
		(*tables)[i].cursor->destroy(&(*tables)[i].cursor);
		ion_err_t error = ion_close_dictionary((*tables)[i].dictionary);

		if (NULL != (*tables)[i].schema) {
			if (NULL != (*tables)[i].schema->field_size)
				free((*tables)[i].schema->field_size);
			if (NULL != (*tables)[i].schema->field_type)
				free((*tables)[i].schema->field_type);
		}
	}
	free(*tables);
	*tables = NULL;
};

void
iinq_destroy_iterator(
		iinq_iterator_t *it
) {
	if (NULL != it->query) {
		iinq_destroy_tuple(&it->query->tuple);
		if (NULL != it->query->filter) {
			for (int i = 0; i < it->query->num_filters; i++) {
				switch (it->query->tables[it->query->filter[i].source_num].schema->field_type[it->query->filter->field_num]) {
					case IINQ_INT:
						if (NULL != it->query->filter[i].comp_value.int_val)
							free(it->query->filter[i].comp_value.int_val);
						break;
					case IINQ_DOUBLE:
						break;
					case IINQ_UINT:
						if (NULL != it->query->filter[i].comp_value.uint_val)
							free(it->query->filter[i].comp_value.uint_val);
						break;
					case IINQ_STRING:
						if (NULL != it->query->filter[i].comp_value.string_val)
							free(it->query->filter[i].comp_value.string_val);
						break;
				}
			}
			free(it->query->filter);
		}
		if (NULL != it->query->sort) {
			ion_external_sort_destroy_cursor(it->query->sort->cursor);
			if (NULL != it->query->sort->record_buf)
				it->query->sort->record_buf;
			free(it->query->sort);
			fremove("orderby");
		}
		if (NULL != it->query->group_by) {
			if (NULL != it->query->group_by->record_buf)
				free(it->query->group_by->record_buf);
			if (NULL != it->query->group_by->cur_key)
				free(it->query->group_by->cur_key);
			if (NULL != it->query->group_by->input_file) {
				fclose(it->query->group_by->input_file);
				fremove("groupby");
			}
			if (NULL != it->query->group_by->old_key)
				free(it->query->group_by->old_key);
		}
		if (NULL != it->query->tables) {
			iinq_destroy_query_tables(&it->query->tables, it->query->num_tables);
		}
	}
}

ion_err_t
iinq_create_table(
		char *schema_file_name,
		ion_key_type_t key_type,
		ion_key_size_t key_size,
		ion_value_size_t value_size,
		iinq_schema_t *schema
) {
	ion_err_t error;
	FILE *schema_file;
	ion_dictionary_t dictionary;
	ion_dictionary_handler_t handler;

	dictionary.handler = &handler;

	error = ion_init_master_table();

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
		if (schema->num_fields !=
			fwrite(schema->field_type, sizeof(iinq_field_type_t), schema->num_fields, schema_file)) {
			return err_file_incomplete_write;
		}

		/* write field sizes in table */
		if (schema->num_fields !=
			fwrite(schema->field_size, sizeof(iinq_field_size_t), schema->num_fields, schema_file)) {
			return err_file_incomplete_write;
		}

		if (0 != fclose(schema_file)) {
			return err_file_close_error;
		}

		ion_close_dictionary(&dictionary);

		error = err_ok;
	} else {
		error = err_file_open_error;
	}

	ion_close_master_table();

	return error;
}

ion_boolean_t
iinq_string_comparison(
		const char *value,
		iinq_comparison_operator_t operator,
		const char *comp_value
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
		int value,
		iinq_comparison_operator_t operator,
		int comp_value
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
		double value,
		iinq_comparison_operator_t operator,
		int comp_value
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
		char *name,
		iinq_table_t *table
) {
	table->dictionary = malloc(sizeof(ion_dictionary_t));

	table->dictionary->handler = malloc(sizeof(ion_dictionary_handler_t));

	if (NULL == table->dictionary->handler) {
		free(table->dictionary);
		return err_out_of_memory;
	}

	/* same code as iinq_open_source(name, table->dictionary, table->dictionary->handler) but more stuff added in */

	ion_err_t error;
	FILE *schema_file;
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

		if (table->schema->num_fields !=
			fread(table->schema->field_type, sizeof(iinq_field_type_t), table->schema->num_fields, schema_file)) {
			free(table->schema->num_fields);
			free(table->schema);
			free(table->dictionary->handler);
			free(table->dictionary);
			return err_file_incomplete_read;
		}

		table->schema->field_size = malloc(sizeof(iinq_field_size_t) * table->schema->num_fields);

		if (table->schema->num_fields !=
			fread(table->schema->field_size, sizeof(iinq_field_size_t), table->schema->num_fields, schema_file)) {
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
	} else {
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
		int num_filters,
		va_list *filters
) {
	it->query->filter = malloc(sizeof(iinq_where_filter_t) * num_filters);

	iinq_where_filter_t *filter;

	filter = va_arg(*filters, iinq_where_filter_t *);

	for (int i = 0; i < num_filters; i++) {
		it->query->filter[i].operator = filter->operator;
		it->query->filter[i].source_num = filter->source_num;
		it->query->filter[i].field_num = filter->field_num;
		void *comp_value = malloc(it->query->tables[0].schema->field_size[filter->field_num]);
		int size = it->query->tables[0].schema->field_size[filter->field_num];
		switch (it->query->tables[0].schema->field_type[filter->field_num]) {
			case IINQ_INT:
				memcpy(comp_value, filter->comp_value.int_val, size);
				it->query->filter[i].comp_value.int_val = comp_value;
				break;
			case IINQ_UINT:
				memcpy(comp_value, filter->comp_value.uint_val, size);
				it->query->filter[i].comp_value.uint_val = comp_value;
				break;
			case IINQ_DOUBLE:
				memcpy(comp_value, filter->comp_value.double_val, size);
				it->query->filter[i].comp_value.double_val = comp_value;
				break;
			case IINQ_STRING:
				memcpy(comp_value, filter->comp_value.string_val, size);
				it->query->filter[i].comp_value.string_val = comp_value;
				break;
		}
		filter++;
	}
}

iinq_field_t
iinq_get_field_from_table(
		iinq_iterator_t *it,
		int table_num,
		int field_num
) {
	iinq_field_t field;

	if (0 == field_num) {
		/* Key was selected */
		field = it->query->tables[table_num].record.key;
	} else {
		/* A field within value was selected */
		field = it->query->tables[table_num].record.value;

		/* loop to the point in the byte array that has the location of the field */
		for (int i = 1; i < field_num; i++) {
			field += it->query->tables->schema->field_size[i];
		}
	}

	return field;
}

iinq_field_t
iinq_get_field_from_sort(
		iinq_iterator_t *it,
		int sort_num,
		int field_num
) {
	iinq_field_t field;

	/* record is located in the byte array just past the sort information */
	field = it->query->sort->record_buf + it->query->sort->size;

	/* loop to the point in the byte array that has the location of the field */
	for (int i = 0; i < field_num; i++) {
		field += it->query->tables->schema->field_size[i];
	}

	return field;
}

iinq_field_t
iinq_get_field_from_group_by(iinq_iterator_t *it, int table_num, int field_num) {

	iinq_field_t field = it->query->group_by->record_buf;

	for (int i = 0; i < field_num; i++) {
		field += it->query->tables->schema->field_size[i];
	}

	return field;
}

ion_boolean_t
iinq_where_from_group_by(
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
				char *value = (char *) iinq_get_field_from_group_by(it, 0, filter->field_num);

				if (!iinq_string_comparison(value, filter->operator, filter->comp_value.string_val)) {
					return boolean_false;
				}

				break;
			}

			case IINQ_INT: {
				int value = NEUTRALIZE(iinq_get_field_from_group_by(it, 0, filter->field_num),
									   int);

				if (!iinq_int_comparison(value, filter->operator, *filter->comp_value.int_val)) {
					return boolean_false;
				}

				break;
			}
			case IINQ_UINT: {
				int value = NEUTRALIZE(iinq_get_field_from_group_by(it, 0, filter->field_num),
									   unsigned int);

				if (!iinq_int_comparison(value, filter->operator, *filter->comp_value.int_val)) {
					return boolean_false;
				}

				break;
			}

			case IINQ_DOUBLE: {
				double value = NEUTRALIZE(iinq_get_field_from_group_by(it, 0, filter->field_num),
										  double);

				if (iinq_double_comparison(value, filter->operator, *filter->comp_value.double_val)) {
					return boolean_false;
				}

				break;
			}
		}
	}

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

				if (!iinq_string_comparison(value, filter->operator, filter->comp_value.string_val)) {
					return boolean_false;
				}

				break;
			}

			case IINQ_INT: {
				int value = NEUTRALIZE(iinq_get_field_from_table(it, 0, filter->field_num),
									   int);

				if (!iinq_int_comparison(value, filter->operator, *filter->comp_value.int_val)) {
					return boolean_false;
				}

				break;
			}

			case IINQ_UINT: {
				int value = NEUTRALIZE(iinq_get_field_from_table(it, 0, filter->field_num),
									   unsigned int);

				if (!iinq_int_comparison(value, filter->operator, *filter->comp_value.int_val)) {
					return boolean_false;
				}

				break;
			}

			case IINQ_DOUBLE: {
				double value = NEUTRALIZE(iinq_get_field_from_table(it, 0, filter->field_num),
										  double);

				if (iinq_double_comparison(value, filter->operator, *filter->comp_value.double_val)) {
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
		if (cs_cursor_active !=
			it->query->tables->cursor->next(it->query->tables->cursor, &it->query->tables->record)) {
			return it->status = it_status_end_of_results;
		} else if (!iinq_where_from_table(it)) {
			continue;
		} else {
			valid = boolean_true;
		}
	}

	for (int i = 0; i < it->query->tables[0].schema->num_fields; i++) {
		it->query->tuple.fields[i] = iinq_get_field_from_table(it, 0, i);
	}

	return it->status;
}

iinq_iterator_status_t
select_field_list_group_by_next(
		iinq_iterator_t *it
) {
	ion_boolean_t valid = boolean_false;

	/* Read the new record */
	if (1 ==
		fread(it->query->group_by->cur_key,
			  it->query->group_by->group_by_size,
			  1,
			  it->query->group_by->input_file) &&
		1 ==
		fread(it->query->group_by->record_buf,
			  it->query->group_by->record_size,
			  1,
			  it->query->group_by->input_file)) {
		return (it->status = it_status_end_of_results);
	}

	iinq_field_list_t *field_list = it->query->tuple.field_list;

	for (int i = 0; i < it->query->tuple.schema->num_fields; i++) {
		int table_num = field_list[i].table_num;
		int field_num = field_list[i].field_num;

		it->query->tuple.fields[i] = iinq_get_field_from_group_by(it, table_num, field_num);
	}

	return it->status;
}

iinq_iterator_status_t
select_all_group_by_next(
		iinq_iterator_t *it
) {
	/* Read the new record */
	if (1 ==
		fread(it->query->group_by->cur_key,
			  it->query->group_by->group_by_size,
			  1,
			  it->query->group_by->input_file) &&
		1 ==
		fread(it->query->group_by->record_buf,
			  it->query->group_by->record_size,
			  1,
			  it->query->group_by->input_file)) {
		return (it->status = it_status_end_of_results);
	}

	char *sort_data = it->query->group_by->record_buf;

	for (int i = 0; i < it->query->tuple.schema->num_fields; i++) {
		it->query->tuple.fields[i] = sort_data;
		sort_data += it->query->tuple.schema->field_size[i];
	}

	return it->status;
}

iinq_iterator_status_t
select_field_list_next(
		iinq_iterator_t *it
) {
	ion_boolean_t valid = boolean_false;

	while (!valid) {
		if (cs_cursor_active !=
			it->query->tables->cursor->next(it->query->tables->cursor, &it->query->tables->record)) {
			return it->status = it_status_end_of_results;
		}

		if (!iinq_where_from_table(it)) {
			continue;
		} else {
			valid = boolean_true;
		}
	}

	iinq_field_list_t *field_list = it->query->tuple.field_list;

	for (int i = 0; i < it->query->tuple.schema->num_fields; i++) {
		int table_num = field_list[i].table_num;
		int field_num = field_list[i].field_num;

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
		it->query->tuple.fields[i] = sort_data;
		sort_data += it->query->tuple.schema->field_size[i];
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
		int table_num = field_list[i].table_num;
		int field_num = field_list[i].field_num;

		it->query->tuple.fields[i] = iinq_get_field_from_sort(it, table_num, field_num);
	}

	return it->status;
}

void
iinq_init_order_by_pointers(iinq_iterator_t *it, iinq_order_by_field_t *order_by_field,
							iinq_order_part_t *orderby_order_parts, iinq_retrieval_location_t location) {

	switch (location) {
		case IINQ_FROM_GROUP_BY:
			orderby_order_parts->pointer = iinq_get_field_from_group_by(it, order_by_field->field_info.table_num,
																		order_by_field->field_info.field_num);
			break;
		case IINQ_FROM_TABLE:
			if (0 == order_by_field->field_info.field_num) {
				orderby_order_parts->pointer = it->query->tables[order_by_field->field_info.table_num].record.key;
			} else {
				orderby_order_parts->pointer = iinq_get_field_from_table(it, order_by_field->field_info.table_num,
																		 order_by_field->field_info.field_num);
			}
			break;
		default:
			break;
	}
}

ion_err_t
iinq_order_by_write_to_file(iinq_iterator_t *it, int orderby_n, iinq_order_part_t *orderby_order_parts,
							iinq_retrieval_location_t location) {
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

	int i;
	int write_page_remaining = IINQ_PAGE_SIZE;
	int total_orderby_size = it->query->sort->size;
	ion_key_size_t key_size = it->query->tables[0].dictionary->instance->record.key_size;
	ion_value_size_t value_size = it->query->tables[0].dictionary->instance->record.value_size;

	/* Filter before sorting.
	 * if we are reading from a table, while condition is tied to the cursor
	 * TODO: create a cursor for reading the group by file
	 * if we are reading from a group by file, while condition is tied to the successful read of the next record in the file*/
	while ((location == IINQ_FROM_TABLE) ? (cs_cursor_active ==
											it->query->tables->cursor->next(it->query->tables->cursor,
																			&it->query->tables->record)) : (1 ==
																											fread(it->query->group_by->cur_key,
																												  it->query->group_by->group_by_size,
																												  1,
																												  it->query->group_by->input_file) &&
																											1 ==
																											fread(it->query->group_by->record_buf,
																												  it->query->group_by->record_size,
																												  1,
																												  it->query->group_by->input_file))) {
		if (!((location == IINQ_FROM_TABLE) ? iinq_where_from_table(it) : iinq_where_from_group_by(it))) {
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
			} else {
				write_page_remaining -= orderby_order_parts[i].size;
			}
		}

		if (1 !=
			fwrite((IINQ_FROM_TABLE == location) ? (it->query->tables->record.key) : (it->query->group_by->record_buf),
				   key_size, 1, file)) {
			break;
		} else {
			write_page_remaining -= key_size;
		}

		if (1 != fwrite((IINQ_FROM_TABLE == location) ? (it->query->tables->record.value) : (
				it->query->group_by->record_buf + key_size), value_size, 1, file)) {
			break;
		} else {
			write_page_remaining -= value_size;
		}
	}

	/* All records have been written, so close file */
	if (NULL != file) {
		fclose(file);
	}
}

iinq_iterator_status_t
iinq_next_from_table_no_predicate(iinq_iterator_t *it) {
	if (cs_cursor_active !=
		it->query->tables->cursor->next(it->query->tables->cursor, &it->query->tables->record)) {
		return it->status = it_status_end_of_results;
	} else
		return it->status = it_status_ok;
}

iinq_iterator_status_t
iinq_next_from_table_with_predicate(iinq_iterator_t *it) {
	ion_boolean_t valid = boolean_false;

	while (!valid) {
		if (cs_cursor_active !=
			it->query->tables->cursor->next(it->query->tables->cursor, &it->query->tables->record)) {
			return it->status = it_status_end_of_results;
		} else if (!it->query->predicate(it)) {
			continue;
		} else {
			return it->status = it_status_ok;
		}
	}
}

iinq_iterator_status_t
iinq_init_table(iinq_iterator_t *it, char *table_name) {
	ion_err_t error;
	// allocate memory for query
	it->query = malloc(sizeof(iinq_query_t));
	if (NULL == it->query) {
		return it->status = it_status_memory_error;
	}
	// allocate memory for table
	it->query->num_tables = 1;
	it->query->tables = malloc(sizeof(iinq_table_t));
	if (NULL == it->query->tables) {
		free(it->query);
		return it->status = it_status_memory_error;
	}
	// allocate memory for table schema
	it->query->tables[0].schema = malloc(sizeof(iinq_schema_t));
	if (NULL == it->query->tables) {
		free(it->query);
		return it->status = it_status_memory_error;
	}
	// open the table for reading
	char file_name[ION_MAX_FILENAME_LENGTH];
	strcpy(file_name, table_name);
	error = iinq_open_table(strcat(file_name, ".inq"), &it->query->tables[0]);
	// allocate memory to hold a key from the table
	it->query->tables->record.key = malloc(it->query->tables->dictionary->instance->record.key_size);
	// allocate memory to hold a value from the table
	it->query->tables->record.value = malloc(it->query->tables->dictionary->instance->record.value_size);
}

iinq_iterator_status_t
iinq_init_tuple_from_table_full_schema(iinq_iterator_t *it) {
	/* Allocate the memory needed for the full schema */
	it->query->tuple.size = 0;
	it->query->tuple.schema = malloc(sizeof(iinq_schema_t));
	it->query->tuple.schema->num_fields = 0;
	it->query->tuple.size += it->query->tables[0].dictionary->instance->record.key_size;
	it->query->tuple.size += it->query->tables[0].dictionary->instance->record.value_size;
	it->query->tuple.schema->num_fields += it->query->tables[0].schema->num_fields;
	it->query->tuple.schema->field_type = malloc(
			sizeof(iinq_field_type_t) * it->query->tables[0].schema->num_fields);
	it->query->tuple.schema->field_size = malloc(
			sizeof(iinq_field_size_t) * it->query->tables[0].schema->num_fields);

	it->query->tuple.fields = malloc(sizeof(iinq_field_t) * it->query->tuple.schema->num_fields);

	/* SELECT * has pointers to every field in the table */
	int i;
	for (i = 0; i < it->query->tuple.schema->num_fields; i++) {
		it->query->tuple.schema->field_type[i] = it->query->tables[0].schema->field_type[i];
		it->query->tuple.schema->field_size[i] = it->query->tables[0].schema->field_size[i];
	}

	// first field is the key
	it->query->tuple.fields[0] = it->query->tables[0].record.key;

	// rest of the fields are in the value of the record
	int byte_index = 0;
	for (i = 1; i < it->query->tuple.schema->num_fields; i++) {
		it->query->tuple.fields[i] = it->query->tables[0].record.value + byte_index;
		byte_index += it->query->tuple.schema->field_size[i];
	}
}

iinq_iterator_status_t
iinq_query_init_select_all_from_table(iinq_iterator_t *it, char *table_name, iinq_iterator_next_t next,
									  iinq_predicate_t predicate) {
	// initialize the table for the query
	iinq_init_table(it, table_name);

	// set next method and predicate
	it->query->predicate = predicate;
	it->next = next;

	// initialize tuple
	iinq_init_tuple_from_table_full_schema(it);

	it->query->sort = NULL;
	it->query->group_by = NULL;
	it->query->filter = NULL;
}

iinq_iterator_status_t
iinq_query_init_select_field_list_from_table(iinq_iterator_t *it, char *table_name, iinq_iterator_next_t next,
											 iinq_predicate_t predicate, int num_fields,
											 iinq_field_list_t *field_list) {
	// initialize the table for the query
	iinq_init_table(it, table_name);

	// set next method and predicate
	it->query->predicate = predicate;
	it->next = next;

	it->query->tuple.size = 0;
	it->query->tuple.schema = malloc(sizeof(iinq_schema_t));
	it->query->tuple.schema->field_type = malloc(sizeof(iinq_field_type_t)*num_fields);
	it->query->tuple.schema->field_size = malloc(sizeof(iinq_field_size_t)*num_fields);
	it->query->tuple.fields = malloc(sizeof(iinq_field_t)*num_fields);
	it->query->tuple.schema->num_fields = num_fields;
	for (int i = 0; i < num_fields; i++) {
		it->query->tuple.size += it->query->tables[field_list[i].table_num].schema->field_size[field_list[i].field_num];
		it->query->tuple.schema->field_type[i] = it->query->tables[field_list[i].table_num].schema->field_type[field_list[i].field_num];
		it->query->tuple.fields[i] = iinq_get_field_from_table(it, field_list[i].table_num, field_list[i].field_num);
	}

	it->query->sort = NULL;
	it->query->group_by = NULL;
	it->query->filter = NULL;
}

iinq_iterator_status_t
iinq_init_order_by(iinq_iterator_t *it, int orderby_n, iinq_order_by_field_t *order_by_field,
				   iinq_retrieval_location_t location) {

	int i;
	ion_err_t error;

	int total_orderby_size = 0;
	iinq_order_part_t *orderby_order_parts = malloc(sizeof(iinq_order_part_t) * orderby_n);

	for (i = 0; i < orderby_n; i++) {
		iinq_init_order_by_pointers(it, &order_by_field[i], orderby_order_parts, location);

		orderby_order_parts[i].direction = order_by_field[i].direction;
		orderby_order_parts[i].size = it->query->tables[order_by_field[i].field_info.table_num].schema->field_size[order_by_field[i].field_info.field_num];

		switch (it->query->tables[order_by_field[i].field_info.table_num].schema->field_type[order_by_field[i].field_info.field_num]) {
			case IINQ_INT:
				orderby_order_parts[i].type = IINQ_ORDERTYPE_INT;
				break;

			case IINQ_UINT:
				orderby_order_parts[i].type = IINQ_ORDERTYPE_UINT;
				break;

			case IINQ_DOUBLE:
				orderby_order_parts[i].type = IINQ_ORDERTYPE_FLOAT;
				break;

			case IINQ_STRING:
				orderby_order_parts[i].type = IINQ_ORDERTYPE_OTHER;
				break;
		}

		total_orderby_size += orderby_order_parts[i].size;
	}

	it->query->sort = malloc(sizeof(iinq_sort_t));
	it->query->sort->size = total_orderby_size;
	int write_page_remaining = IINQ_PAGE_SIZE;
	ion_key_size_t key_size = it->query->tables[0].dictionary->instance->record.key_size;
	ion_value_size_t value_size = it->query->tables[0].dictionary->instance->record.value_size;

	iinq_order_by_write_to_file(it, orderby_n, orderby_order_parts, location);

	/* Open file in read mode for sorting */
	FILE *file = fopen("orderby", "rb");

	iinq_sort_context_t *context = malloc(sizeof(iinq_sort_context_t));

	*context = _IINQ_SORT_CONTEXT(orderby);

	ion_external_sort_t *es = malloc(sizeof(ion_external_sort_t));

	error = ion_external_sort_init(es, file, context, iinq_sort_compare, key_size + value_size,
								   key_size + value_size + total_orderby_size, IINQ_PAGE_SIZE, boolean_false,
								   ION_FILE_SORT_FLASH_MINSORT);

	uint16_t buffer_size = ion_external_sort_bytes_of_memory_required(es, 0, boolean_false);

	char *buffer = malloc(buffer_size);

	/* recordbuf needs enough room for the sort field and the table tuple (sort field is stored twice)
	 * projection is done afterward */
	it->query->sort->record_buf = malloc((total_orderby_size + key_size + value_size));

	it->query->sort->cursor = malloc(sizeof(ion_external_sort_cursor_t));

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

iinq_iterator_status_t
init(
		iinq_iterator_t *it,
		iinq_select_type_t select_type,
		iinq_order_by_type_t order_by_type,
		ion_boolean_t has_group_by,
		int num_tables,
		int num_filters,
		...
) {
	int i;
	va_list list;

	va_start(list, num_filters);

	it->query = malloc(sizeof(iinq_query_t));
	/* multiply sizeof(ion_iinq_table_t) by number of dictionaries used in query */
	it->query->tables = malloc(sizeof(iinq_table_t) * num_tables);
	it->query->num_tables = num_tables;

	if (NULL == it->query->tables) {
		return it->status = it_status_invalid;
	}

	ion_err_t error;

	for (i = 0; i < num_tables; i++) {
		char name[ION_MAX_FILENAME_LENGTH];

		strcpy(name, va_arg(list, char *));
		it->query->tables[i].schema = malloc(sizeof(iinq_schema_t));
		error = iinq_open_table(strcat(name, ".inq"), &it->query->tables[i]);

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
	} else {
		it->query->filter = NULL;
	}

	it->query->select_type = select_type;
	it->query->tuple.schema = malloc(sizeof(iinq_schema_t));

	/* if SELECT field_list */
	if (IINQ_SELECT_FIELD_LIST == select_type) {
		/* get the number of fields to be selected */
		it->query->tuple.schema->num_fields = va_arg(list, int);
		/* allocate the appropriate number of pointers for the data */
		it->query->tuple.fields = malloc(sizeof(iinq_field_t) * it->query->tuple.schema->num_fields);
		it->query->tuple.schema->field_type = malloc(sizeof(iinq_field_type_t)*it->query->tuple.schema->num_fields);
		it->query->tuple.schema->field_size = malloc(sizeof(iinq_field_size_t)*it->query->tuple.schema->num_fields);

		if (has_group_by && IINQ_ORDER_BY_NONE == order_by_type) {
			it->next = select_field_list_group_by_next;
		} else if (IINQ_ORDER_BY_NONE == order_by_type) {
			it->next = select_field_list_next;
		} else if (IINQ_ORDER_BY_FIELD == order_by_type) {
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
		it->query->tuple.size = 0;
		field_list = va_arg(list, iinq_field_list_t *);

		for (i = 0; i < it->query->tuple.schema->num_fields; i++) {
			it->query->tuple.field_list[i].table_num = field_list->table_num;
			it->query->tuple.field_list[i].field_num = field_list->field_num;
			it->query->tuple.size += it->query->tables[field_list->table_num].schema->field_size[field_list->field_num];
			field_list++;
		}
	} else {
		/* else SELECT * */
		it->query->tuple.is_select_all = boolean_true;
		it->query->tuple.field_list = NULL;
		it->query->tuple.size = 0;
		it->query->tuple.schema->num_fields = 0;
		it->query->tuple.size += it->query->tables[0].dictionary->instance->record.key_size;
		it->query->tuple.size += it->query->tables[0].dictionary->instance->record.value_size;
		it->query->tuple.schema->num_fields += it->query->tables[0].schema->num_fields;
		it->query->tuple.schema->field_type = malloc(
				sizeof(iinq_field_type_t) * it->query->tables[0].schema->num_fields);
		it->query->tuple.schema->field_size = malloc(
				sizeof(iinq_field_size_t) * it->query->tables[0].schema->num_fields);

		/* SELECT * has pointers to every field in the table */
		for (i = 0; i < it->query->tables[0].schema->num_fields; i++) {
			it->query->tuple.schema->field_type[i] = it->query->tables[0].schema->field_type[i];
			it->query->tuple.schema->field_size[i] = it->query->tables[0].schema->field_size[i];
		}

		it->query->tuple.fields = malloc(sizeof(iinq_field_t) * it->query->tuple.schema->num_fields);

		if (IINQ_ORDER_BY_NONE == order_by_type) {
			it->next = select_all_next;
		} else if (IINQ_ORDER_BY_FIELD == order_by_type) {
			it->next = select_all_order_by_next;
		}
	}

	/* if GROUP BY clause */
	if (has_group_by) {
		FILE *output_file;
		FILE *input_file;
		int groupby_n = va_arg(list,
							   int);
		iinq_field_list_t *group_by_info = va_arg(list, iinq_field_list_t *);
		int total_groupby_size = 0;
		iinq_order_part_t *groupby_order_parts = malloc(sizeof(iinq_order_part_t) * groupby_n);

		for (i = 0; i < groupby_n; i++) {
			if (0 == group_by_info[i].field_num) {
				groupby_order_parts[i].pointer = it->query->tables[group_by_info[i].table_num].record.key;
			} else {
				groupby_order_parts[i].pointer = iinq_get_field_from_table(it, group_by_info[i].table_num,
																		   -group_by_info[i].field_num);
			}
			/* Original code needed a direction for the GROUP BY to do the sorting. Using external sort needs the direction. */
			groupby_order_parts[i].direction = 1;
			groupby_order_parts[i].size = it->query->tables[group_by_info[i].table_num].schema->field_size[group_by_info[i].field_num];

			switch (it->query->tables[group_by_info[i].table_num].schema->field_type[group_by_info[i].field_num]) {
				case IINQ_INT:
					groupby_order_parts[i].type = IINQ_ORDERTYPE_INT;
					break;

				case IINQ_UINT:
					groupby_order_parts[i].type = IINQ_ORDERTYPE_UINT;
					break;

				case IINQ_DOUBLE:
					groupby_order_parts[i].type = IINQ_ORDERTYPE_FLOAT;
					break;

				case IINQ_STRING:
					groupby_order_parts[i].type = IINQ_ORDERTYPE_OTHER;
					break;
			}

			total_groupby_size += groupby_order_parts[i].size;
			output_file = fopen("groupby", "wb");
			if (NULL == output_file) {
				ion_close_dictionary(it->query->tables);
				free(it->query->tables);
				free(it->query->tables->record.key);
				free(it->query->tables->record.value);
				free(it->query->tuple.fields);
				it->status = it_status_invalid;
				return it;
			}
		}

		int write_page_remaining = IINQ_PAGE_SIZE;
		ion_key_size_t key_size = it->query->tables[0].dictionary->instance->record.key_size;
		ion_value_size_t value_size = it->query->tables[0].dictionary->instance->record.value_size;

		/* Write the group by field, key and value to group by file for each record */
		while (cs_cursor_active ==
			   it->query->tables->cursor->next(it->query->tables->cursor, &it->query->tables->record)) {
			if (!iinq_where_from_table(it)) {
				continue;
			}

			/* If there is not enough space on the page, fill the page with zeroes and go to the next one. */
			if (write_page_remaining < (total_groupby_size + key_size + value_size)) {
				char x = 0;

				for (i = 0; i < write_page_remaining; i++) {
					if (1 != fwrite(&x, 1, 1, output_file)) {
						break;
					}
				}

				write_page_remaining = IINQ_PAGE_SIZE;
			}

			for (i = 0; i < groupby_n; i++) {
				if (1 != fwrite(groupby_order_parts[i].pointer, groupby_order_parts[i].size, 1, output_file)) {
					break;
				} else {
					write_page_remaining -= groupby_order_parts[i].size;
				}
			}

			if (1 != fwrite(it->query->tables->record.key, key_size, 1, output_file)) {
				break;
			} else {
				write_page_remaining -= key_size;
			}

			if (1 != fwrite(it->query->tables->record.value, value_size, 1, output_file)) {
				break;
			} else {
				write_page_remaining -= value_size;
			}
		}

		/* Close the group by file */
		if (0 != fclose(output_file)) {
			it->status = it_status_invalid;
			return it;
		}

		/* Destroy the cursor for the table and close the table (we access records through other files now). */
		it->query->tables[0].cursor->destroy(it->query->tables[0].cursor);
		ion_close_dictionary(it->query->tables[0].dictionary);
		FREE_AND_NULL(it->query->tables->record.key);
		FREE_AND_NULL(it->query->tables->record.value);

		/* Open the group by file for read. */
		input_file = fopen("groupby", "rb");
		if (NULL == input_file) {
			ion_close_dictionary(it->query->tables);
			free(it->query->tables);
			free(it->query->tables->record.key);
			free(it->query->tables->record.value);
			free(it->query->tuple.fields);
			it->status = it_status_invalid;
			return it;
		}

		/* Create a file to store the sorted results */
		output_file = fopen("sortedgb", "wb");
		if (NULL == output_file) {
			ion_close_dictionary(it->query->tables);
			free(it->query->tables);
			free(it->query->tables->record.key);
			free(it->query->tables->record.value);
			free(it->query->tuple.fields);
			it->status = it_status_invalid;
			return it;
		}

		iinq_sort_context_t *context = malloc(sizeof(iinq_sort_context_t));
		*context = _IINQ_SORT_CONTEXT(groupby);
		ion_external_sort_t *es = malloc(sizeof(ion_external_sort_t));

		error = ion_external_sort_init(es, input_file, &context, iinq_sort_compare, key_size + value_size,
									   key_size + value_size + total_groupby_size, IINQ_PAGE_SIZE, boolean_false,
									   ION_FILE_SORT_FLASH_MINSORT);

		/* Dump all of the records into the sorted file */
		uint16_t buffer_size = ion_external_sort_bytes_of_memory_required(es, 0, boolean_true);
		char *buffer = malloc(buffer_size);
		ion_external_sort_dump_all(es, output_file, buffer, buffer_size);
		free(buffer);

		/* Close the input and output files */
		fclose(input_file);
		fclose(output_file);

		/* Open the sorted file for reading */
		input_file = fopen("sortedgb", "rb");
		it->query->group_by->input_file = input_file;

		it->query->group_by = malloc(sizeof(iinq_group_by_t));

		/* These will contain GROUP BY fields to be compared */
		char *cur_key = malloc(total_groupby_size);
		char *old_key = malloc(total_groupby_size);
		it->query->group_by->cur_key = cur_key;
		it->query->group_by->old_key = old_key;

		it->query->group_by->record_size = key_size + value_size;

		/* Allocate space to read a record from the sorted file */
		char *record_buf = malloc(sizeof(key_size + value_size));
		it->query->group_by->record_buf = record_buf;

		/* End of init if there is no ORDER BY */

		/* If there is an ORDER BY clause */
		if (IINQ_ORDER_BY_FIELD == order_by_type) {
			output_file = fopen("orderby", "wb");
			/* initialize order by */
			int orderby_n = va_arg(list, int);
			iinq_order_by_field_t *order_by_field = va_arg(list, iinq_order_by_field_t*);
			iinq_init_order_by(it, orderby_n, order_by_field, IINQ_FROM_GROUP_BY);

			/* There will be additional code to be run after the first record */
			ion_boolean_t is_first = boolean_true;

			while (1) {
				if (1 != fread(cur_key, total_groupby_size, 1, input_file)) {
					fclose(input_file);
					it->status = it_status_invalid;
					return it;
				}

				if (1 != fread(record_buf, key_size + value_size, 1, input_file)) {
					fclose(input_file);
					it->status = it_status_invalid;
					return it;
				}

				/* Aggregates would be initialized here in the macro */

				if (!is_first) {
					if (A_equ_B != iinq_sort_compare(context, cur_key, old_key)) {
						/* Copy order by fields into order by pointers */

						/* If (!HAVING) continue; */

						/* Write pointers to file */

						/* Write aggregates to file */

						/* Write record to file */

						/* Reset aggregates */
					}
				}

				/* TODO: fix to allow multiple group by fields */
				memcpy(groupby_order_parts[0].pointer, cur_key, sizeof(cur_key));

				/* We are moving to the next record */
				memcpy(old_key, cur_key, total_groupby_size);
				is_first = boolean_false;
			}

			/* If at least one record was read */
			if (!is_first) {
				/* copy into order by pointers */
				/* Should be done already when iinq_order_by_init is called */

				/* if (!HAVING) clean up aggregates */

				/* Write order by to file */

				/* Write aggregates to file */

				/* Write record to file */

				/* Close input file (sortedgb) */

				/* Close output file (orderby) */

				/* Open orderby for read (input file) */

				/* Next method uses order by */

			}

		}
		/* else group by is used in next method */

	}
		/* if ORDER BY attribute without GROUP BY */
	else if (IINQ_ORDER_BY_NONE != order_by_type) {
		if (IINQ_ORDER_BY_FIELD == order_by_type) {
			int orderby_n = va_arg(list, int);
			iinq_order_by_field_t *order_by_field = va_arg(list, iinq_order_by_field_t *);
			/* No GROUP BY so we are sorting directly from the table */
			iinq_init_order_by(it, orderby_n, order_by_field, IINQ_FROM_TABLE);
			it->query->group_by = NULL;
		}
	} else {
		it->query->sort = NULL;
		it->query->group_by = NULL;
	}

	it->status = it_status_ok;

	va_end(list);

	return it;
}