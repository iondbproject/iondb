#include "iinq_rewrite.h"
#include "../iinq/iinq.h"

ion_err_t
iinq_rewrite_open_source(
	char						*name,
	ion_iinq_rewrite_source_t	*source
) {
	ion_err_t error;

	source->dictionary = malloc(sizeof(ion_dictionary_t));

	strcpy(source->name, name);

	if (source->name == NULL) {
		return err_out_of_memory;
	}

	source->dictionary->handler = malloc(sizeof(ion_dictionary_handler_t));

	if (NULL == source->dictionary->handler) {
		free(source->name);
		return err_out_of_memory;
	}

	error = iinq_open_source(name, source->dictionary, source->dictionary->handler);

	if (err_ok != error) {
		free(source->dictionary->handler);
		free(source->name);
		return error;
	}

	ion_predicate_t predicate;

	error = dictionary_build_predicate(&predicate, predicate_all_records);

	if (err_ok != error) {
		free(source->dictionary->handler);
		free(source->name);
		return error;
	}

	error = dictionary_find(source->dictionary, &predicate, &source->cursor);

	if (err_ok != error) {
		free(source->dictionary->handler);
		free(source->name);
	}

	return error;
}

ion_iinq_tuple_t *
select_all_next(
	ion_iinq_iterator_t it
) {
	if (cs_cursor_active != it->sources->cursor->next(it->sources->cursor, &it->sources->record)) {
		it->iterator_status = it_status_invalid;
		return NULL;
	}

	it->tuple.fields = it->sources->record.value;
	return &it->tuple;
}

ion_iinq_tuple_t *
select_field_list_next(
	ion_iinq_iterator_t it
) {
	if (cs_cursor_active != it->sources->cursor->next(it->sources->cursor, &it->sources->record)) {
		it->iterator_status = it_status_invalid;
		return NULL;
	}

	DEFINE_SCHEMA(test1, {
		double col1;
		char col2[40];
		int col3;
		char col4[10];
	});

	struct iinq_test1_schema *test1_tuple;

	test1_tuple			= (struct iinq_test1_schema *) it->sources->record.value;

	it->tuple.size		= 0;

	it->tuple.fields[0] = &test1_tuple->col1;
	it->tuple.size		+= sizeof(test1_tuple->col1);

	it->tuple.fields[1] = &test1_tuple->col2;
	it->tuple.size		+= sizeof(test1_tuple->col2);

	it->tuple.fields[2] = &test1_tuple->col3;
	it->tuple.size		+= sizeof(test1_tuple->col3);

	return &it->tuple;
}

ion_iinq_tuple_t *
select_field_list_order_by_next(
	ion_iinq_iterator_t it
) {
	ion_err_t error = it->sort_cursor->next(it->sort_cursor, it->recordbuf);

	if (err_ok != error) {
		it->iterator_status = it_status_invalid;
		return NULL;
	}

	DEFINE_SCHEMA(test1, {
		double col1;
		char col2[40];
		int col3;
		char col4[10];
	});

	struct iinq_test1_schema *test1_tuple;

	ion_iinq_tuple_data_t	data		= it->tuple.fields;
	char					*sort_data	= it->recordbuf + sizeof(uint32_t);

	*data		= *sort_data;	/* col1 */
	data++;
	sort_data	+= sizeof(test1_tuple->col1);
	*data		= *sort_data;	/* col2 */
	data++;
	sort_data	+= sizeof(test1_tuple->col2);
	*data		= *sort_data;	/* col3 */
	data++;
	sort_data	+= sizeof(test1_tuple->col3);
}

ion_iinq_iterator_t
init(
	ion_boolean_t	is_field_list,
	ion_boolean_t	is_order_by
) {
	ion_iinq_iterator_t it = malloc(sizeof(struct iterator));

	/* multiply sizeof(ion_iinq_rewrite_source_t) by number of dictionaries used in query */
	it->sources = malloc(sizeof(ion_iinq_rewrite_source_t));
	;

	if (NULL == it->sources) {
		it->iterator_status = it_status_invalid;
		return it;
	}

	ion_err_t error;

	error = iinq_rewrite_open_source("test1.inq", it->sources);

	/* predicate used to look through table,
	 * for each source with multiple tables */
	if (err_ok != error) {
		it->iterator_status = it_status_invalid;
		return it;
	}

	it->sources->record.key = malloc(it->sources->dictionary->instance->record.key_size);

	if (NULL == it->sources->record.key) {
		ion_close_dictionary(it->sources);
		free(it->sources);
		it->iterator_status = it_status_invalid;
		return it;
	}

	it->sources->record.value = malloc(it->sources->dictionary->instance->record.value_size);

	if (NULL == it->sources->record.value) {
		ion_close_dictionary(it->sources);
		free(it->sources);
		free(it->sources->record.key);
		it->iterator_status = it_status_invalid;
		return it;
	}

	/* if SELECT field_list */
	if (is_field_list) {
		it->tuple.num_fields	= 3;
		it->tuple.fields		= malloc(sizeof(void *) * it->tuple.num_fields);

		it->next				= select_field_list_next;

		if (NULL == it->tuple.fields) {
			ion_close_dictionary(it->sources);
			free(it->sources);
			free(it->sources->record.key);
			free(it->sources->record.value);
			it->iterator_status = it_status_invalid;
			return it;
		}
	}
	else {
		it->next = select_all_next;
	}

	/* if ORDER BY attribute */
	if (is_order_by) {
		DEFINE_SCHEMA(test1, {
			double col1;
			char col2[40];
			int col3;
			char col4[10];
		});

		struct iinq_test1_schema	*test1_tuple;
		int							orderby_n				= 1;
		int							total_orderby_size		= 0;
		iinq_order_part_t			*orderby_order_parts	= alloca(sizeof(iinq_order_part_t));

		orderby_order_parts[0].pointer		= &test1_tuple->col3;
		orderby_order_parts[0].direction	= 1;
		orderby_order_parts[0].size			= sizeof(test1_tuple->col3);
		orderby_order_parts[0].type			= IINQ_ORDERTYPE_INT;
		total_orderby_size					+= orderby_order_parts[0].size;

		it->tuple.size						= 0;
		it->tuple.size						+= sizeof(test1_tuple->col1);
		it->tuple.size						+= sizeof(test1_tuple->col2);
		it->tuple.size						+= sizeof(test1_tuple->col3);
		it->tuple.size						+= sizeof(test1_tuple->col4);

		FILE *file = fopen("orderby", "wb");

		if (NULL == file) {
			ion_close_dictionary(it->sources);
			free(it->sources);
			free(it->sources->record.key);
			free(it->sources->record.value);
			free(it->tuple.fields);
			it->iterator_status = it_status_invalid;
			return it;
		}

		int write_page_remaining = IINQ_PAGE_SIZE;

		/* cursor already initialized via iinq_rewrite_open_source */
		while (cs_cursor_active == it->sources->cursor->next(it->sources->cursor, &it->sources->record)) {
			for (int i = 0; i < orderby_n; i++) {
				if (1 != fwrite(orderby_order_parts[i].pointer, orderby_order_parts[i].size, 1, file)) {
					break;
				}
				else {
					write_page_remaining -= orderby_order_parts[i].size;
				}

				if (1 != fwrite(&it->sources->record, it->tuple.size, 1, file)) {
					break;
				}
				else {
					write_page_remaining -= it->tuple.size;
				}
			}
		}

		if (NULL != file) {
			fclose(file);
		}

		file = fopen("orderby", "rb");

		if (NULL == file) {
			ion_close_dictionary(it->sources);
			free(it->sources);
			free(it->sources->record.key);
			free(it->sources->record.value);
			free(it->tuple.fields);
			it->iterator_status = it_status_invalid;
			return it;
		}

		iinq_sort_context_t context = _IINQ_SORT_CONTEXT(orderby);
		ion_external_sort_t *es		= malloc(sizeof(ion_external_sort_t));

		error = ion_external_sort_init(es, file, &context, iinq_sort_compare, it->tuple.size, it->tuple.size + total_orderby_size, IINQ_PAGE_SIZE, boolean_false, ION_FILE_SORT_FLASH_MINSORT);

		uint16_t buffer_size = ion_external_sort_bytes_of_memory_required(es, 0, boolean_false);

		it->buffer		= malloc(buffer_size);
		/* recordbuf needs enough room for the key as well */
		it->recordbuf	= malloc((total_orderby_size + it->tuple.size) + sizeof(uint32_t));

		it->sort_cursor = malloc(sizeof(ion_external_sort_cursor_t));

		if (err_ok != (error = ion_external_sort_init_cursor(es, &it->sort_cursor, it->buffer, buffer_size))) {
			ion_close_dictionary(it->sources);
			free(it->sources);
			free(it->sources->record.key);
			free(it->sources->record.value);
			free(it->tuple.fields);
			it->iterator_status = it_status_invalid;
			fclose(file);
			return it;
		}
	}

	it->iterator_status = it_status_ok;
	return it;
}
