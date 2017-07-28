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
SQL_create(
	ion_table_t *table,
	char		*sql
) {
	table = malloc(sizeof(ion_table_t));

	char *substring = sql + 13;

	printf("%s\n", substring);

	int		pos;
	char	*pointer = strstr(substring, " ");

	pos = (int) (pointer - substring);

	char table_name[20];

	memcpy(table_name, substring, pos);
	strncat(table_name, ".inq", 4);
	table_name[strlen(table_name)]	= '\0';

	table->table_name				= table_name;
	printf("table: %s\n", table->table_name);

	substring						= pointer + 2;
	printf("%s\n", table_name);

	/* Calculate number of fields in table */
	int i, count;

	for (i = 0, count = 0; substring[i]; i++) {
		count += (substring[i] == ',');
	}

	printf("%i\n", count);
	printf("%s\n", substring);

	ion_attribute_t table_fields[count];
	ion_key_type_t	key_type;

	/* Set up attribute names and types */
	for (int j = 0; j < count; j++) {
		printf("%s\n", substring);
		pointer = strstr(substring, ",");

		pos		= (int) (pointer - substring);

		char field[pos + 1];

		memcpy(field, substring, pos);
		field[pos]	= '\0';

		substring	= pointer + 2;

		pointer		= strstr(field, " ");

		pos			= (int) (pointer - field);

		char field_name[pos + 1];

		memcpy(field_name, field, pos);
		field_name[pos]				= '\0';

		table_fields[j].field_name	= field_name;

		char field_type[strlen(field) - strlen(field_name) + 1];

		memcpy(field_type, &field[pos + 1], strlen(field) + 1);
		field_type[strlen(field) - strlen(field_name)]	= '\0';

		key_type										= ion_switch_key_type(field_type);
		table_fields[j].field_type						= key_type;

		printf("%s.\n", table_fields[j].field_name);
		printf("%s.\n", field_type);
	}

	printf("%s\n", substring);

	/* Table set-up */

	pointer = strstr(substring, "(");

	pos		= (int) (pointer - substring);

	char primary_key[pos];

	memcpy(primary_key, pointer + 1, strlen(pointer) - 3);
	primary_key[strlen(pointer) - 3] = '\0';

	printf("%s.\n", primary_key);

	/* Set up table for primary key */

	ion_key_size_t	primary_key_size;
	ion_key_type_t	primary_key_type;

	for (int j = 0; j < count; j++) {
		/* Primary key attribute information found */
		if (0 == strncmp(primary_key, table_fields[j].field_name, strlen(primary_key))) {
			primary_key_type = table_fields[j].field_type;
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

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	error = iinq_create_source(table_name, primary_key_type, primary_key_size, value_size);

	if (err_ok != error) {
		printf("Error occurred creating table. Error code: %i\n", error);
	}

	dictionary.handler	= &handler;

	error				= iinq_open_source(table_name, &dictionary, &handler);

	if (err_ok != error) {
		printf("Error occurred opening table. Error code: %i\n", error);
	}
}

void
SQL_insert(
	ion_table_t *table,
	char		*sql
) {}

void
SQL_update(
	ion_table_t *table,
	char		*sql
) {}

void
SQL_delete(
	ion_table_t *table,
	char		*sql
) {}

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
