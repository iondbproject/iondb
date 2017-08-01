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

#include "iinq_execute.h"

ion_key_type_t
ion_switch_key_type(
	char *key_type
) {
	if (0 == strncmp(key_type, "CHAR", 4)) {
		return key_type_char_array;
	}

	if (0 == strncmp(key_type, "VARCHAR", 7)) {
		return key_type_null_terminated_string;
	}

	if (0 == strncmp(key_type, "INT", 3)) {
		return key_type_numeric_signed;
	}

	return key_type_numeric_signed;
}

int
ion_switch_key_size(
	ion_key_type_t key_type
) {
	switch (key_type) {
		case key_type_numeric_signed: {
			return sizeof(int);
		}

		case key_type_numeric_unsigned: {
			return sizeof(int);
		}

		case key_type_char_array: {
			return sizeof("ABRACADABRA");
		}

		case key_type_null_terminated_string: {
			return sizeof("ABRACADABRA");
		}
	}
}

void
print_table(
	ion_table_t			*table,
	ion_dictionary_t	*dictionary
) {
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(table->key_size);
	ion_record.value	= malloc(table->value_size);

	char table_name[strlen(table->table_name) - 3];

	memcpy(table_name, table->table_name, strlen(table->table_name) - 3);
	table_name[strlen(table->table_name) - 4] = '\0';

	printf("\nTable: %s\n", table_name);

	for (int i = 0; i < table->num_fields; i++) {
		printf("%s\t", table->table_fields[i].field_name);

		if (0 == strncmp("age", table->table_fields[i].field_name, 3)) {
			printf("\t");
		}
	}

	printf("\n***************************************\n");

	ion_cursor_status_t cursor_status;

	while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
		printf("%s\n", (char *) ion_record.value);
	}

	cursor->destroy(&cursor);
	free(ion_record.key);
	free(ion_record.value);
}

void
SQL_create(
	ion_table_t *table,
	char		*sql
) {
	printf("\n%s\n", sql);

	char *substring		= sql + 13;

	int		pos;
	char	*pointer	= strstr(substring, " ");

	pos = (int) (pointer - substring);

	char table_name[ION_MAX_FILENAME_LENGTH];

	memcpy(table_name, substring, pos);

	snprintf(table->table_name, ION_MAX_FILENAME_LENGTH, "%s.%s", table_name, "inq");

	substring			= pointer + 2;

	table->num_records	= 0;

	/* Calculate number of fields in table */
	int i, count;

	for (i = 0, count = 0; substring[i]; i++) {
		count += (substring[i] == ',');
	}

	ion_attribute_t table_fields[count];
	ion_key_type_t	key_type;

	/* Set up attribute names and types */
	for (int j = 0; j < count; j++) {
		/* Initialize attribute struct */
		table_fields[j].field_name	= malloc(sizeof("ABRACADABRA"));

		pointer						= strstr(substring, ",");

		pos							= (int) (pointer - substring);

		char field[pos + 1];

		memcpy(field, substring, pos);
		field[pos]	= '\0';

		substring	= pointer + 2;

		pointer		= strstr(field, " ");

		pos			= (int) (pointer - field);

		char field_name[pos + 1];

		memcpy(field_name, field, pos);
		field_name[pos] = '\0';

		strcpy(table_fields[j].field_name, field_name);

		char field_type[strlen(field) - strlen(field_name) + 1];

		memcpy(field_type, &field[pos + 1], strlen(field) + 1);
		field_type[strlen(field) - strlen(field_name)]	= '\0';

		key_type										= ion_switch_key_type(field_type);
		table_fields[j].field_type						= key_type;

		table->table_fields[j]							= table_fields[j];
	}

	/* Table set-up */

	pointer = strstr(substring, "(");

	pos		= (int) (pointer - substring);

	char primary_key[pos];

	memcpy(primary_key, pointer + 1, strlen(pointer) - 3);
	primary_key[strlen(pointer) - 3] = '\0';

	/* Set up table for primary key */

	ion_key_size_t	primary_key_size;
	ion_key_type_t	primary_key_type;

	for (int j = 0; j < count; j++) {
		/* Primary key attribute information found */
		if ((0 == strncmp(primary_key, table_fields[j].field_name, strlen(primary_key))) && (strlen(primary_key) == strlen(table_fields[j].field_name))) {
			primary_key_type			= table_fields[j].field_type;
			table->primary_key_field	= j;
		}
	}

	primary_key_size = ion_switch_key_size(primary_key_type);

	ion_value_size_t value_size = 0;

	for (int j = 0; j < count; j++) {
		value_size += ion_switch_key_size(table_fields[j].field_type);
	}

	table->key_type		= primary_key_type;
	table->key_size		= primary_key_size;
	table->value_size	= value_size;
	table->num_fields	= count;

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	error = iinq_create_source(table->table_name, primary_key_type, primary_key_size, value_size);

	if (err_ok != error) {
		printf("Error occurred creating table. Error code: %i\n", error);
	}

	dictionary.handler	= &handler;

	error				= iinq_open_source(table->table_name, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error occurred opening table. Error code: %i\n", error);
	}

	print_table(table, &dictionary);

	/* Close table */
	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error occurred closing table.\n");
	}
}

void
SQL_insert(
	ion_table_t *table,
	char		*sql
) {
	printf("\n%s\n", sql);

	char *substring = sql + 12 + (strlen(table->table_name) - 3);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table->table_name, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error occurred opening table. Error code: %i\n", error);
	}

	char *pointer = strstr(substring, "(");

	substring = pointer + 1;

	char value[strlen(substring) - 2];

	memcpy(value, substring, strlen(substring) - 1);
	value[strlen(substring) - 1] = '\0';

	ion_key_t	key;
	int			pos;

	/* Get key value from record to be inserted */
	for (int j = 0; j < table->num_fields; j++) {
		if (j == table->primary_key_field) {
			pointer = strstr(substring, ",");

			if (NULL == pointer) {
				pointer = strstr(substring, ")");
			}

			if (NULL == pointer) {
				printf("Error occurred inserting values, please check that a value has been listed for each column in table.\n");
			}

			pos = (int) (pointer - substring);

			char field[pos + 1];

			memcpy(field, substring, pos);
			field[pos]	= '\0';

			substring	= pointer + 2;

			/* If column value is an INT, ionize it to be added to the table */
			if (0 == table->table_fields[j].field_type) {
				int num = atoi(field);

				key = IONIZE(num, int);
			}
			else {
				key = malloc(strlen(field));
				strcpy(key, field);
			}

			break;
		}
	}

	printf("Record inserted: %s\n", value);

	ion_status_t status = dictionary_insert(&dictionary, key, value);

	if (err_ok != status.error) {
		printf("Error occurred inserting record into table.\n");
	}

	table->num_records++;

	print_table(table, &dictionary);

	/* Close table */
	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error occurred closing table.\n");
	}
}

ion_compare_type_t
compare_type(
	char	*sql,
	char	*ret_pointer
) {
	char *pointer;

	pointer = strstr(sql, "!=");

	if (NULL != pointer) {
		strcpy(ret_pointer, pointer);
		return ion_not_equal;
	}

	pointer = strstr(sql, "=");

	if (NULL != pointer) {
		strcpy(ret_pointer, pointer);
		return ion_equal;
	}

	pointer = strstr(sql, "<=");

	if (NULL != pointer) {
		strcpy(ret_pointer, pointer);
		return ion_less_than_equal;
	}

	pointer = strstr(sql, "<");

	if (NULL != pointer) {
		strcpy(ret_pointer, pointer);
		return ion_less_than;
	}

	pointer = strstr(sql, ">=");

	if (NULL != pointer) {
		strcpy(ret_pointer, pointer);
		return ion_greater_than_equal;
	}

	pointer = strstr(sql, ">");

	if (NULL != pointer) {
		strcpy(ret_pointer, pointer);
		return ion_greater_than;
	}
	else {
		return ion_invalid_comparison;
	}
}

ion_boolean_t
condition_satified(
	ion_table_t		*table,
	char			*sql,
	ion_record_t	*record
) {
	int					pos;
	char				*pointer	= malloc(sizeof(sql));
	ion_compare_type_t	compare		= compare_type(sql, pointer);

	if (ion_invalid_comparison == compare) {
		printf("Error occurred! Please check that you have entered a valid WHERE condition.\n");
		return boolean_false;
	}

	if (NULL != pointer) {
		pos = (int) (strlen(sql) - strlen(pointer));

		char	field[pos];
		char	*substring;
		char	*value;

		memcpy(field, sql, pos);
		field[pos] = '\0';

		int len;

		for (int j = 0; j < table->num_fields; j++) {
			/* Column to evaluate WHERE condition found */
			if ((0 == strncmp(field, table->table_fields[j].field_name, strlen(field))) && (strlen(field) == strlen(table->table_fields[j].field_name))) {
				if ((compare == ion_not_equal) || (compare == ion_less_than_equal) || (compare == ion_greater_than_equal)) {
					sql = sql + (strlen(field) + 2);
					len = (int) (strlen(sql));
				}
				else {
					sql = sql + strlen(field) + 1;
					len = (int) strlen(sql);
				}

				int		num;
				char	val[len];

				memcpy(val, sql, len);
				val[len - 1] = '\0';

				/* Column to be evaluated is the primary key */
				if (j == table->primary_key_field) {
					num		= get_int(record->key);

					value	= malloc(strlen(record->key));
					strcpy(value, record->key);
				}
				/* Separate record into column to be evaluated */
				else {
					substring = malloc(strlen(record->value));
					strcpy(substring, record->value);

					/* Get field value for column j as it is the column to be evaluated */
					for (int i = 0; i <= j; i++) {
						pointer = strstr(substring, ",");

						if (NULL == pointer) {
							char col_val[strlen(substring)];

							memcpy(col_val, substring, strlen(substring) + 1);
							col_val[strlen(substring)]	= '\0';

							value						= malloc(strlen(col_val));
							strcpy(value, col_val);
						}
						else {
							pos = (int) (pointer - substring);

							char col_val[pos + 1];

							memcpy(col_val, substring, pos);
							col_val[pos]	= '\0';

							substring		= pointer + 2;

							value			= malloc(strlen(col_val));
							strcpy(value, col_val);
						}
					}
				}

				/* If field type to be evaluated is INT */
				if (0 == table->table_fields[j].field_type) {
					num = atoi(value);

					if (((ion_not_equal == compare) && (num != atoi(val))) || ((ion_equal == compare) && (num == atoi(val))) || ((ion_less_than == compare) && (num < atoi(val))) || ((ion_less_than_equal == compare) && (num <= atoi(val))) || ((ion_greater_than == compare) && (num > atoi(val))) || ((ion_greater_than_equal == compare) && (num >= atoi(val)))) {
						return boolean_true;
					}
					else {
						return boolean_false;
					}
				}
				/* Else it is of type CHAR[] or VARCHAR */
				else if (((ion_not_equal == compare) && (0 != strncmp(val, value, strlen(val)))) || ((ion_equal == compare) && (0 == strncmp(val, value, strlen(val)))) || ((ion_less_than == compare) && (0 > strncmp(val, value, strlen(val)))) || ((ion_less_than_equal == compare) && (0 >= strncmp(val, value, strlen(val)))) || ((ion_greater_than == compare) && (0 < strncmp(val, value, strlen(val)))) || ((ion_greater_than_equal == compare) && (0 <= strncmp(val, value, strlen(val))))) {
					return boolean_true;
				}
				else {
					return boolean_false;
				}
			}
		}
	}

	printf("Error occurred! Please check that you have entered a valid WHERE condition.\n");
	return boolean_false;
}

void
SQL_update(
	ion_table_t *table,
	char		*sql
) {
	printf("\n%s\n", sql);

	char *substring = sql + 11 + (strlen(table->table_name) - 3);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table->table_name, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error occurred opening table. Error code: %i\n", error);
	}

	ion_boolean_t	where_condition = boolean_false;
	char			*pointer		= strstr(substring, "WHERE");
	char			*where;

	int len;

	if (NULL != pointer) {
		len = (int) (strlen(substring) - strlen(pointer));
	}
	else {
		len = (int) strlen(substring);
	}

	char update_fields[len];

	if (NULL != pointer) {
		where_condition = boolean_true;

		char field[strlen(pointer) - 6];

		memcpy(field, pointer + 6, strlen(pointer) - 6);
		field[strlen(pointer) - 6]	= '\0';

		where						= malloc(strlen(substring) - strlen(pointer));
		strcpy(where, field);

		strncpy(update_fields, substring, strlen(substring) - strlen(pointer));
		update_fields[strlen(substring) - strlen(pointer) - 1] = '\0';
	}

	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(&dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(table->key_size);
	ion_record.value	= malloc(table->value_size);

	ion_boolean_t where_satisfied = boolean_false;

	pointer = strstr(update_fields, "=");

	char update_field[strlen(update_fields) - strlen(pointer)];

	strncpy(update_field, update_fields, strlen(update_fields) - strlen(pointer));
	update_field[strlen(update_fields) - strlen(pointer)] = '\0';

	char *update_value = update_fields + strlen(update_field) + 1;

	int				pos;
	char			*old_record, *record;
	ion_key_t		key;
	ion_boolean_t	update_key = boolean_false;
	ion_status_t	status;

	ion_cursor_status_t cursor_status;

	while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
		if (where_condition) {
			where_satisfied = condition_satified(table, where, &ion_record);
		}

		if (!where_condition || (boolean_true == where_satisfied)) {
			/* Initialize old record value */
			char	value[strlen(ion_record.value) + 1];
			char	*field_value;

			memcpy(value, ion_record.value, strlen(ion_record.value));
			value[strlen(ion_record.value)] = '\0';

			record							= malloc(strlen(value));
			strcpy(record, value);

			old_record						= malloc(strlen(record));
			strcpy(old_record, record);

			/* Parse record to find fields to update */
			for (int j = 0; j < table->num_fields; j++) {
				/* Column to UPDATE found */
				if ((0 == strncmp(update_field, table->table_fields[j].field_name, strlen(update_field))) && (strlen(update_field) == strlen(table->table_fields[j].field_name))) {
					/* Get old column value to remove */
					/* Get field value for column j as it is the column to be evaluated */
					for (int i = 0; i <= j; i++) {
						pointer = strstr(record, ",");

						if (NULL == pointer) {
							char col_val[strlen(record)];

							memcpy(col_val, record, strlen(record) + 1);
							col_val[strlen(record)] = '\0';

							field_value				= malloc(strlen(col_val));
							strcpy(field_value, col_val);
						}
						else {
							pos = (int) (pointer - record);

							char col_val[pos + 1];

							memcpy(col_val, record, pos);
							col_val[pos]	= '\0';

							record			= pointer + 2;

							field_value		= malloc(strlen(col_val));
							strcpy(field_value, col_val);
						}
					}

					/* If value being updated is the primary key, update it as well */
					if (j == table->primary_key_field) {
						/* If column value is an INT, ionize it to be added to the table */
						if (0 == table->table_fields[j].field_type) {
							status = dictionary_delete(&dictionary, IONIZE(atoi(field_value), int));

							if (err_ok != status.error) {
								printf("Error occurred updating record in table.\n");
							}

							update_key = boolean_true;

							int num = atoi(update_value);

							key = IONIZE(num, int);
						}
						else {
							key = malloc(strlen(update_value));
							strcpy(key, update_value);
						}
					}
					/* Else key remains unchanged */
					else {
						key = ion_record.key;
					}

					pointer = strstr(old_record, field_value);

					char new_record[table->value_size];

					pos = (int) (pointer - old_record);

					memcpy(new_record, old_record, pos);

					len = (int) strlen(new_record);

					strcat(new_record, update_value);
					strcat(new_record, old_record + len + strlen(field_value));

					printf("Updated record: %s\n", new_record);

					if (update_key) {
						status = dictionary_insert(&dictionary, key, new_record);
					}
					else {
						status = dictionary_update(&dictionary, key, new_record);
					}

					if (err_ok != status.error) {
						printf("Error occurred updating record in table.\n");
					}
				}
			}
		}
	}

	cursor->destroy(&cursor);
	free(ion_record.key);
	free(ion_record.value);

	print_table(table, &dictionary);

	/* Close table */
	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error occurred closing table.\n");
	}
}

void
SQL_delete(
	ion_table_t *table,
	char		*sql
) {
	printf("\n%s\n", sql);

	char *substring = sql + 12 + (strlen(table->table_name) - 3);

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	dictionary.handler	= &handler;

	error				= iinq_open_source(table->table_name, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error occurred opening table. Error code: %i\n", error);
	}

	ion_boolean_t	where_condition = boolean_false;
	char			*pointer		= strstr(substring, "WHERE");
	char			*where;

	int len;

	if (NULL != pointer) {
		len = (int) (strlen(substring) - strlen(pointer));
	}
	else {
		len = (int) strlen(substring);
	}

	char update_fields[len];

	if (NULL != pointer) {
		where_condition = boolean_true;

		memcpy(substring, substring, 4);

		char field[strlen(pointer) - 6];

		memcpy(field, pointer + 6, strlen(pointer) - 6);
		field[strlen(pointer) - 5]	= '\0';

		len							= (int) (strlen(substring) - strlen(pointer));
		where						= malloc((size_t) len);
		strcpy(where, field);

		strncpy(update_fields, substring, len);
		update_fields[len - 1] = '\0';
	}

	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(&dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(table->key_size);
	ion_record.value	= malloc(table->value_size);

	ion_boolean_t where_satisfied = boolean_false;

	ion_status_t status;

	ion_cursor_status_t cursor_status;
	int					count = 0;

	ion_record_t deleted_records[table->num_records];

	for (int i = 0; i < table->num_records; i++) {
		deleted_records[i].key		= malloc(table->key_size);
		deleted_records[i].value	= malloc(table->value_size);
	}

	while ((cursor_status = cursor->next(cursor, &deleted_records[count])) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
		if (where_condition) {
			where_satisfied = condition_satified(table, where, &deleted_records[count]);
		}

		if (where_condition && (boolean_false == where_satisfied)) {
			deleted_records[count].key		= NULL;
			deleted_records[count].value	= NULL;
		}

		count++;
	}

	for (int i = 0; i < count; i++) {
		if (NULL != deleted_records[i].key) {
			status = dictionary_delete(&dictionary, deleted_records[i].key);

			if (err_ok != status.error) {
				printf("Error occurred deleting record from table.\n");
				break;
			}

			printf("Record deleted: %s\n", (char *) deleted_records[i].value);
			table->num_records--;
		}
	}

	print_table(table, &dictionary);

	/* Close table */
	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error occurred closing table.\n");
	}
}

void
SQL_drop(
	ion_table_t *table,
	char		*sql
) {}

void
SQL_execute(
	ion_table_t *table,
	char		*sql
) {
	char *command = strstr(sql, "CREATE TABLE");

	if (NULL != command) {
		SQL_create(table, sql);
		return;
	}

	command = strstr(sql, "INSERT INTO");

	if (NULL != command) {
		SQL_insert(table, sql);
		return;
	}

	command = strstr(sql, "UPDATE");

	if (NULL != command) {
		SQL_update(table, sql);
		return;
	}

	command = strstr(sql, "DELETE");

	if (NULL != command) {
		SQL_delete(table, sql);
		return;
	}

	command = strstr(sql, "DROP TABLE");

	if (NULL != command) {
		SQL_drop(table, sql);
		return;
	}

	printf("SQL Command was not recognized.\n");
}
