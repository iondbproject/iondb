/******************************************************************************/
/**
@file		iinq_user.c
@author		Dana Klamut
@brief		This code contains definitions for iinq user functions
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#include "iinq_user.h"

void
uppercase(
	char	*string,
	char	uppercase[]
) {
	int i;
	int len = (int) strlen(string);

	for (i = 0; i < len; i++) {
		uppercase[i] = (char) toupper(string[i]);
	}

	uppercase[i] = '\0';
}

char *
next_keyword(
	char *keyword
) {
	if (0 == strncmp("SELECT", keyword, 6)) {
		return "FROM";
	}

	if (0 == strncmp("FROM", keyword, 4)) {
		return "WHERE";
	}

	if (0 == strncmp("WHERE", keyword, 5)) {
		return "ORDERBY";
	}

	if (0 == strncmp("ORDERBY", keyword, 7)) {
		return "GROUPBY";
	}

	if (0 == strncmp("GROUPBY", keyword, 7)) {
		return NULL;
	}
	else {
		return NULL;
	}
}

ion_err_t
get_clause(
	char	*keyword,
	char	sql[],
	char	clause[]
) {
	char	*end_pointer, *start_pointer, *curr_key;
	int		start_pos, end_pos;

	/* Get position of next keyword (if exists) */
	curr_key = next_keyword(keyword);

	while (1) {
		if (NULL != curr_key) {
			end_pointer = strstr(sql, curr_key);

			if (NULL != end_pointer) {
				break;
			}
		}
		else {
			end_pointer = NULL;
			break;
		}

		curr_key = next_keyword(curr_key);
	}

	start_pointer = strstr(sql, keyword);

	if (NULL == start_pointer) {
		return err_uninitialized;
	}

	if (NULL == end_pointer) {
		end_pos		= (int) (strlen(sql) + 1);
		start_pos	= (int) (start_pointer - sql);
	}
	else {
		end_pos		= (int) (end_pointer - sql);
		start_pos	= (int) (start_pointer - sql);
	}

	/* Get clause */
	memcpy(clause, &sql[start_pos], end_pos - 1);
	clause[(end_pos - start_pos) - 1] = '\0';

	return err_ok;
}

void
get_field_list(
	char	*keyword,
	char	clause[],
	char	field_list[]
) {
	memcpy(field_list, &clause[(int) strlen(keyword) + 1], (int) strlen(clause));
}

void
table_cleanup(
	ion_query_iterator_t *iterator
) {
	ion_dictionary_id_t id = iterator->cursor->dictionary->instance->id;

	/* Table clean-up */
	char cleanup_name[20];

	sprintf(cleanup_name, "%d.bpt", (int) id);
	fremove(cleanup_name);
	sprintf(cleanup_name, "%d.val", (int) id);
	fremove(cleanup_name);
	fremove("TEST.INQ");
}

int
get_int(
	void *value
) {
	return NEUTRALIZE(value, int);
}

ion_record_t
get_largest(
	ion_query_iterator_t *iterator,
	ion_record_t max_record
) {
	ion_dictionary_t			*dictionary = malloc(sizeof(ion_dictionary_t));

	dictionary = iterator->dictionary;

	ion_predicate_t predicate;
	int num_records = 0;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = malloc(sizeof(ion_dict_cursor_t));

	dictionary_find(dictionary, &predicate, &cursor);

	/* Initialize iterator */
	ion_record_t curr_record;
	curr_record.key = malloc((size_t) dictionary->instance->record.key_size);
	curr_record.value = malloc((size_t) dictionary->instance->record.value_size);

	ion_cursor_status_t cursor_status;

	cursor_status = cursor->next(cursor, &curr_record);

	if((cs_cursor_active != cursor_status) && (cs_cursor_initialized != cursor_status)) {
		printf("Error!");
	}

	max_record = curr_record;

	while ((cursor_status = cursor->next(cursor, &curr_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
		if(get_int(curr_record.key) > get_int(max_record.key)) {
			max_record = curr_record;
		}

		num_records++;
	}

	dictionary_delete(dictionary, max_record.key);

	cursor->destroy(&cursor);
	free(curr_record.key);
	free(curr_record.value);
	free(dictionary);

	iterator->num_records = num_records;

	return max_record;
}

ion_record_t
next(
	ion_query_iterator_t	*iterator,
	ion_record_t			record
) {
	ion_cursor_status_t cursor_status;

	ion_record_t largest_record;
	largest_record.key	= malloc((size_t) iterator->dictionary->instance->record.key_size);
	largest_record.value	= malloc((size_t) iterator->dictionary->instance->record.value_size);

	if((iterator->where_condition) && (iterator->orderby_condition)) {
		largest_record = get_largest(iterator, largest_record);

		if(0 < iterator->num_records) {
			if (3 > get_int(largest_record.key)) {
				return largest_record;
			}
		}
	}

	else {
		while (((cursor_status = iterator->cursor->next(iterator->cursor, &record)) ==
				cs_cursor_active) || (cursor_status == cs_cursor_initialized)) {
			/* Evaluate WHERE condition */
			if ((iterator->where_condition) && !(iterator->orderby_condition)) {
				if (3 > get_int(record.key)) {
					/* Return the retrieved record that meets WHERE condition */
					return record;
				}
			}

				/* No WHERE or ORDERBY or GROUPBY condition - return any retrieved record */
			else {
				return record;
			}
		}
	}

	record.key		= NULL;
	record.value	= NULL;

	table_cleanup(iterator);
	iterator->destroy(iterator);

	return record;
}

void
destroy(
	ion_query_iterator_t *iterator
) {
	iterator->cursor->destroy(&iterator->cursor);
	free(iterator->record.key);
	free(iterator->record.value);
}

void
table_setup(
) {
	/* Table set-up */
	char				*schema_file_name;
	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	schema_file_name	= "TEST.INQ";
	key_type			= key_type_numeric_signed;
	key_size			= sizeof(int);
	value_size			= sizeof(int);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	error = iinq_create_source(schema_file_name, key_type, key_size, value_size);

	if (err_ok != error) {
		printf("Error1\n");
	}

	dictionary.handler	= &handler;

	error				= iinq_open_source(schema_file_name, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error2\n");
	}

	/* Insert values into table */
	ion_key_t	key1	= IONIZE(1, int);
	ion_value_t value1	= IONIZE(100, int);
	ion_key_t	key2	= IONIZE(2, int);
	ion_value_t value2	= IONIZE(200, int);
	ion_key_t	key3	= IONIZE(3, int);
	ion_value_t value3	= IONIZE(300, int);

	ion_status_t status = dictionary_insert(&dictionary, key1, value1);

	if (err_ok != status.error) {
		printf("Error3\n");
	}

	status = dictionary_insert(&dictionary, key2, value2);

	if (err_ok != status.error) {
		printf("Error4\n");
	}

	status = dictionary_insert(&dictionary, key3, value3);

	if (err_ok != status.error) {
		printf("Error5\n");
	}

	/* Close table and clean-up files */
	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error6\n");
	}
}

ion_err_t
SQL_query(
	ion_query_iterator_t	*iterator,
	char					*sql_string
) {
	char uppercase_sql[(int) strlen(sql_string)];

	uppercase(sql_string, uppercase_sql);
	printf("%s\n", uppercase_sql);

	char		select_clause[50];
	ion_err_t	err					= get_clause("SELECT", uppercase_sql, select_clause);
	char		select_fields[40]	= "null";

	iterator->where_condition = boolean_false;
	iterator->orderby_condition = boolean_false;
	iterator->groupby_condition = boolean_false;
	ion_boolean_t dict_initialized = boolean_false;

	if (err == err_ok) {
		get_field_list("SELECT", select_clause, select_fields);
	}

	char from_clause[50];

	err = get_clause("FROM", uppercase_sql, from_clause);

	char from_fields[40] = "null";

	if (err == err_ok) {
		get_field_list("FROM", from_clause, from_fields);
	}

	char where_clause[50];
	char where_fields[40] = "null";

	err = get_clause("WHERE", uppercase_sql, where_clause);

	if (err == err_ok) {
		get_field_list("WHERE", where_clause, where_fields);
		iterator->where_condition = boolean_true;
	}

	/* Evaluate FROM here */
	ion_dictionary_t			*dictionary = malloc(sizeof(ion_dictionary_t));
	ion_dictionary_handler_t	handler;

	/* currently only supports one table */
	char *schema_file_name = from_fields;

	dictionary->handler = &handler;

	ion_err_t error = iinq_open_source(schema_file_name, dictionary, &handler);

	if (err_ok != error) {
		printf("Error7\n");
	}

	iterator->schema_file_name = from_fields;

	/* Set-up ORDERBY clause if exists */
	char orderby_clause[50];
	char orderby_fields[40] = "null";
	ion_dictionary_t            *orderby_dictionary = malloc(sizeof(ion_dictionary_t));
	ion_dictionary_handler_t	*orderby_handler	= malloc(sizeof(ion_dictionary_handler_t));

	err = get_clause("ORDERBY", uppercase_sql, orderby_clause);

	if (err == err_ok) {
		get_field_list("ORDERBY", orderby_clause, orderby_fields);
		iterator->orderby_condition = boolean_true;

//		sldict_init(&orderby_handler);
//		dictionary_create(&orderby_handler,
//						  orderby_dictionary,
//						  -1,
//						  dictionary->instance->key_type,
//						  dictionary->instance->record.key_size,
//						  dictionary->instance->record.value_size,
//						  20);

		orderby_dictionary = dictionary;
		ffdict_init(orderby_handler);
		orderby_dictionary->handler = orderby_handler;
		dict_initialized = boolean_true;
	}

	/* Set-up GROUPBY clause if exists */
	char groupby_clause[50];
	char groupby_fields[40] = "null";

	err = get_clause("GROUPBY", uppercase_sql, groupby_clause);

	if (err == err_ok) {
		get_field_list("GROUPBY", groupby_clause, groupby_fields);
		iterator->groupby_condition = boolean_true;

		/* If dictionary for grouping has not yet been initialized through ORDERBY,
		   initialize it */
		if(!dict_initialized) {
//			sldict_init(&orderby_handler);
//			dictionary_create(&orderby_handler,
//							  orderby_dictionary,
//							  -1,
//							  dictionary->instance->key_type,
//							  dictionary->instance->record.key_size,
//							  dictionary->instance->record.value_size,
//							  20);
		}

		orderby_dictionary = dictionary;
	}

	/* Evaluate SELECT here */
	ion_predicate_t predicate;

	if (0 == strncmp("*", select_fields, 1)) {
		dictionary_build_predicate(&predicate, predicate_all_records);
	}

	ion_dict_cursor_t *cursor = malloc(sizeof(ion_dict_cursor_t));

	dictionary_find(dictionary, &predicate, &cursor);

	/* Initialize iterator */
	iterator->record.key	= malloc((size_t) dictionary->instance->record.key_size);
	iterator->record.value	= malloc((size_t) dictionary->instance->record.value_size);
	iterator->cursor		= cursor;
	iterator->dictionary	= orderby_dictionary;
	iterator->next			= next;
	iterator->destroy		= destroy;

	free(dictionary);

	return err_ok;
}

int
main(
	void
) {
	table_setup();

	/* Query */
	ion_query_iterator_t iterator;

	SQL_query(&iterator, "Select * FRoM test.inq where Key < 3 orDerby key DESC");

	/* Iterate through results */
	iterator.record = next(&iterator, iterator.record);

	while (iterator.record.key != NULL) {
		printf("Key: %i, Value: %i\n", get_int(iterator.record.key), get_int(iterator.record.value));

		iterator.record = next(&iterator, iterator.record);
	}

	return 0;
}
