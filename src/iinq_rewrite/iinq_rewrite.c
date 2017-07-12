#include "iinq_rewrite.h"

ion_err_t
iinq_rewrite_open_source(
		char *name,
		ion_iinq_rewrite_source_t *source
) {
	ion_err_t error;

	strcpy(source->name, name);
	if (source->name == NULL) {
		return err_out_of_memory;
	}

	source->dictionary.handler = malloc(sizeof(ion_dictionary_handler_t));

	if (NULL == source->dictionary.handler) {
		free(source->name);
		return err_out_of_memory;
	}

	error = iinq_open_source(name, &(source->dictionary), source->dictionary.handler);
	if (err_ok != error) {
		free(source->dictionary.handler);
		free(source->name);
		return error;
	}

	ion_predicate_t predicate;
	error = dictionary_build_predicate(&predicate, predicate_all_records);
	if (err_ok != error) {
		free(source->dictionary.handler);
		free(source->name);
		return error;
	}

	error = dictionary_find(&(source->dictionary), &predicate, &source->cursor);
	if (err_ok != error) {
		free(source->dictionary.handler);
		free(source->name);
	}

	return error;
}

ion_iinq_tuple_t *
select_all_next(ion_iinq_iterator_t it) {
	if (cs_cursor_active != it->sources->cursor->next(it->sources->cursor, &it->sources->record))
		it->iterator_status = it_status_invalid;
	it->tuple.fields = it->sources->record.value;
	return &it->tuple;
}

ion_iinq_tuple_t *
select_field_list_next(ion_iinq_iterator_t it) {
	if(cs_cursor_active != it->sources->cursor->next(it->sources->cursor, &it->sources->record)) {
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
	test1_tuple = (struct iinq_test1_schema*) it->sources->record.value;
	ion_iinq_tuple_data_t data = it->tuple.fields;

	it->tuple.size = 0;
	*data = &test1_tuple->col1;
	it->tuple.size += sizeof(test1_tuple->col1);
	data++;
	*data = &test1_tuple->col2;
	it->tuple.size += sizeof(test1_tuple->col2);
	data++;
	*data = &test1_tuple->col3;
	it->tuple.size += sizeof(test1_tuple->col3);

	return &it->tuple;
}

ion_iinq_iterator_t
init(ion_boolean_t field_list) {
	ion_iinq_iterator_t it = malloc(sizeof(ion_iinq_iterator_t));

	// multiply sizeof(ion_iinq_rewrite_source_t) by number of dictionaries used in query
	ion_iinq_rewrite_source_t *sources = malloc(sizeof(ion_iinq_rewrite_source_t));
	it->sources = sources;

	if (NULL == sources) {
		it->iterator_status = it_status_invalid;
		return it;
	}

	ion_err_t error;
	/* Open dictionary and initialize handler,
	 * for each source with multiple tables */
	ion_dictionary_handler_t handler;
	sources->dictionary.handler = &handler;
	error = iinq_rewrite_open_source("test1.inq", sources);
	/* predicate used to look through table,
	 * for each source with multiple tables */
	if (err_ok != error) {
		it->iterator_status = it_status_invalid;
		return it;
	}

	it->sources->record.key 	= malloc(it->sources->dictionary.instance->record.key_size);

	if (NULL == it->sources->record.key) {
		ion_close_dictionary(sources);
		free(sources);
		it->iterator_status = it_status_invalid;
		return it;
	}

	it->sources->record.value 	= malloc(it->sources->dictionary.instance->record.value_size);

	if (NULL == it->sources->record.value) {
		ion_close_dictionary(sources);
		free(sources);
		free(it->sources->record.key);
		it->iterator_status = it_status_invalid;
		return it;
	}

	/* if SELECT field_list */
	if (field_list) {
		it->tuple.num_fields = 3;
		it->tuple.fields = malloc(sizeof(void *) * it->tuple.num_fields);

		it->next = select_field_list_next;

		if (NULL == it->tuple.fields) {
			ion_close_dictionary(sources);
			free(sources);
			free(it->sources->record.key);
			free(it->sources->record.value);
			it->iterator_status = it_status_invalid;
			return it;
		}
	} else {
		it->next = select_all_next;
	}

	it->iterator_status = it_status_ok;
	return it;
}



