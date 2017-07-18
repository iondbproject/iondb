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
#include <string.h>

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

	sprintf(cleanup_name, "%d.ffs", (int) id);
	fremove(cleanup_name);
	fremove("ion_mt.tbl");
	fremove("TEST1.INQ");
	fremove("TEST2.INQ");
}

int
get_int(
	void *value
) {
	return NEUTRALIZE(value, int);
}

void
sort(
	ion_dictionary_t	*dictionary,
	ion_record_t		records[3]
) {
	ion_predicate_t predicate;
	int				num_records = 0;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = malloc(sizeof(ion_dict_cursor_t));

	dictionary_find(dictionary, &predicate, &cursor);

	/* Initialize records */
	records[0].key		= malloc((size_t) dictionary->instance->record.key_size);
	records[0].value	= malloc((size_t) dictionary->instance->record.value_size);
	records[1].key		= malloc((size_t) dictionary->instance->record.key_size);
	records[1].value	= malloc((size_t) dictionary->instance->record.value_size);
	records[2].key		= malloc((size_t) dictionary->instance->record.key_size);
	records[2].value	= malloc((size_t) dictionary->instance->record.value_size);

	ion_cursor_status_t cursor_status;

	ion_record_t max_key;

	while (3 > num_records) {
		while ((cursor_status = cursor->next(cursor, &records[num_records])) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
			num_records++;
		}
	}

	/* Sort records in array ASC */
	/* Modify for total number of records in table */
	for (int i = 0; i < 3; ++i) {
		for (int j = i + 1; j < 3; ++j) {
			if (get_int(records[i].key) > get_int(records[j].key)) {
				max_key		= records[i];
				records[i]	= records[j];
				records[j]	= max_key;
			}
		}
	}

	cursor->destroy(&cursor);
}

ion_record_t
next_sorted_record(
	ion_record_t	sorted_records[],
	ion_record_t	record
) {
	int pos = 0;

	while (3 > pos) {
		if (NULL != sorted_records[pos].key) {
			record					= sorted_records[pos];

			/* Set record to NULL as not to return it repeatedly */
			sorted_records[pos].key = NULL;
			return record;
		}

		pos++;
	}

	/* Array is empty - all records have been previously returned */
	record.key = NULL;
	return record;
}

ion_record_t
next(
	ion_query_iterator_t	*iterator,
	ion_record_t			record
) {
	ion_cursor_status_t cursor_status;

	/* Set-up for SELECT fieldlist condition */
	if ((iterator->select_fieldlist) && !(iterator->orderby_condition)) {
		while (((cursor_status = iterator->cursor->next(iterator->cursor, &record)) == cs_cursor_active) || (cursor_status == cs_cursor_initialized)) {
			/* Evaluate WHERE condition */
			if ((iterator->where_condition) && !(iterator->orderby_condition)) {
				if (3 > get_int(record.key)) {
					/* Return the retrieved record that meets WHERE condition */
					record.value = (char *) (record.value + 3);
					return record;
				}
			}
			/* No WHERE or ORDERBY or GROUPBY condition - return any retrieved record */
			else {
				record.value = (char *) (record.value + 3);
				return record;
			}
		}
	}

	/* Evaluate ORDERBY condition */
	if (iterator->orderby_condition) {
		record = next_sorted_record(iterator->sorted_records, record);

		while (NULL != (record.key)) {
			/* If WHERE condition, evaluate */
			if (iterator->where_condition) {
				if (3 > get_int(record.key)) {
					if (iterator->select_fieldlist) {
						record.value = (char *) (record.value + 3);
					}

					return record;
				}
			}
			else {
				if (iterator->select_fieldlist) {
					record.value = (char *) (record.value + 3);
				}

				return record;
			}

			record = next_sorted_record(iterator->sorted_records, record);
		}
	}
	else {
		while (((cursor_status = iterator->cursor->next(iterator->cursor, &record)) == cs_cursor_active) || (cursor_status == cs_cursor_initialized)) {
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
/*	if(iterator->orderby_condition) { */
/*		free(iterator->sorted_records[0].key); */
/*		free(iterator->sorted_records[0].value); */
/*		free(iterator->sorted_records[1].key); */
/*		free(iterator->sorted_records[1].value); */
/*		free(iterator->sorted_records[2].key); */
/*		free(iterator->sorted_records[2].value); */
/*	} */

	iterator->cursor->destroy(&iterator->cursor);
	free(iterator->record.key);
/*	free(iterator->record.value); */
}

void
table_setup(
) {
	/* Table set-up */
	char				*schema_file_name;
	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	schema_file_name	= "TEST1.INQ";
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
	ion_key_t	key1	= IONIZE(3, int);
	ion_value_t value1	= IONIZE(300, int);
	ion_key_t	key2	= IONIZE(2, int);
	ion_value_t value2	= IONIZE(200, int);
	ion_key_t	key3	= IONIZE(1, int);
	ion_value_t value3	= IONIZE(100, int);

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

void
fieldlist_table_setup(
) {
	/* Table set-up */
	char				*schema_file_name;
	ion_key_type_t		key_type;
	ion_key_size_t		key_size;
	ion_value_size_t	value_size;

	schema_file_name	= "TEST2.INQ";
	key_type			= key_type_numeric_signed;
	key_size			= sizeof(int);
	value_size			= sizeof("one") + sizeof("two");

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
	ion_key_t	key1	= IONIZE(3, int);
	ion_value_t value1	= "onetwo";
	ion_key_t	key2	= IONIZE(2, int);
	ion_value_t value2	= "thrfor";
	ion_key_t	key3	= IONIZE(1, int);
	ion_value_t value3	= "fivsix";

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

	iterator->where_condition	= boolean_false;
	iterator->orderby_condition = boolean_false;
	iterator->groupby_condition = boolean_false;
	iterator->select_fieldlist	= boolean_false;

	if (err == err_ok) {
		get_field_list("SELECT", select_clause, select_fields);
	}

	char from_clause[50];

	err = get_clause("FROM", uppercase_sql, from_clause);

	char from_fields[40] = "null";

	if (err == err_ok) {
		get_field_list("FROM", from_clause, from_fields);
	}

	char	where_clause[50];
	char	where_fields[40] = "null";

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
	char	orderby_clause[50];
	char	orderby_fields[40] = "null";

	err = get_clause("ORDERBY", uppercase_sql, orderby_clause);

	if (err == err_ok) {
		get_field_list("ORDERBY", orderby_clause, orderby_fields);
		iterator->orderby_condition = boolean_true;

		ion_record_t sorted_records[3];

		sort(dictionary, sorted_records);

		iterator->sorted_records[0] = sorted_records[0];
		iterator->sorted_records[1] = sorted_records[1];
		iterator->sorted_records[2] = sorted_records[2];
	}

	/* Set-up GROUPBY clause if exists */
	char	groupby_clause[50];
	char	groupby_fields[40] = "null";

	err = get_clause("GROUPBY", uppercase_sql, groupby_clause);

	if (err == err_ok) {
		get_field_list("GROUPBY", groupby_clause, groupby_fields);
		iterator->groupby_condition = boolean_true;
	}

	/* Evaluate SELECT here */
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	if (0 != strncmp("*", select_fields, 1)) {
		/* If query is not SELECT * then it is SELECT fieldlist */
		iterator->select_fieldlist = boolean_true;
	}

	ion_dict_cursor_t *cursor = malloc(sizeof(ion_dict_cursor_t));

	dictionary_find(dictionary, &predicate, &cursor);

	/* Initialize iterator */
	iterator->record.key	= malloc((size_t) dictionary->instance->record.key_size);
	iterator->record.value	= malloc((size_t) dictionary->instance->record.value_size);
	iterator->cursor		= cursor;
	iterator->next			= next;
	iterator->destroy		= destroy;

	free(dictionary);

	return err_ok;
}

int
main(
	void
) {
	/* For SELECT * queries */
	table_setup();

	/* For SELECT fieldlist queries */
/*	fieldlist_table_setup(); */

	/* Query */
	ion_query_iterator_t iterator;

/*	SQL_query(&iterator, "Select * FRoM test1.inq"); */
/*	SQL_query(&iterator, "Select * FRoM test1.inq where Key < 3"); */
/*	SQL_query(&iterator, "Select * FRoM test1.inq where Key < 3 orDerby key ASC"); */
	SQL_query(&iterator, "Select * FROM test1.inq orderby key ASC");

/*	SQL_query(&iterator, "Select key, col2 FRoM test2.inq"); */
/*	SQL_query(&iterator, "Select key, col2 FRoM test2.inq where Key < 3"); */
/*	SQL_query(&iterator, "Select key, col2 FRoM test2.inq where Key < 3 orderby key ASC"); */
/*	SQL_query(&iterator, "Select key, col2 FRoM test2.inq orderby key ASC"); */

	/* Iterate through results */
	iterator.record = next(&iterator, iterator.record);

	while (iterator.record.key != NULL) {
		printf("Key: %i ", get_int(iterator.record.key));

		if (iterator.select_fieldlist) {
			printf("Value: %s\n", (char *) (iterator.record.value));
		}
		else {
			printf("Value: %i\n", get_int(iterator.record.value));
		}

		iterator.record = next(&iterator, iterator.record);
	}

	return 0;
}
