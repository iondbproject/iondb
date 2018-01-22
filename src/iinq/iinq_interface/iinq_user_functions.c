#include "iinq_user_functions.h"

void
print_table_dogs(
	ion_dictionary_t *dictionary
) {
	ion_predicate_t predicate;

	dictionary_build_predicate(&predicate, predicate_all_records);

	ion_dict_cursor_t *cursor = NULL;

	dictionary_find(dictionary, &predicate, &cursor);

	ion_record_t ion_record;

	ion_record.key		= malloc(sizeof(int));
	ion_record.value	= malloc(sizeof(int) + sizeof(char) * 20 + sizeof(char) * 30 + sizeof(int) + sizeof(char) * 30);

	printf("Table: Dogs\n");
	printf("id	");
	printf("type	");
	printf("name	");
	printf("age	");
	printf("city	");
	printf("\n***************************************\n");

	ion_cursor_status_t cursor_status;
	unsigned char		*value;

	while ((cursor_status = cursor->next(cursor, &ion_record)) == cs_cursor_active || cursor_status == cs_cursor_initialized) {
		value	= ion_record.value;

		printf("%i	", NEUTRALIZE(value, int));
		value	+= sizeof(int);

		printf("%s	", (char *) value);
		value	+= sizeof(char) * 20;

		printf("%s	", (char *) value);
		value	+= sizeof(char) * 30;

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
create_table1(
) {
	printf("%s\n\n", "CREATE TABLE Dogs (id INT, type CHAR[20], name VARCHAR[30], age INT, city VARCHAR[30], primary key(id))");

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	error = iinq_create_source("Dogs.inq", 0, sizeof(int), sizeof(int) + sizeof(char) * 20 + sizeof(char) * 30 + sizeof(int) + sizeof(char) * 30);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	dictionary.handler	= &handler;
	error				= iinq_open_source("Dogs.inq", &dictionary, &handler);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	print_table_dogs(&dictionary);
	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}
}

iinq_prepared_sql
SQL_prepare(
	char *sql
) {
	iinq_prepared_sql p = { 0 };

	p.setInt	= setInt;
	p.execute	= execute;
	p.value		= malloc(sizeof(int) + sizeof(char) * 20 + sizeof(char) * 30 + sizeof(int) + sizeof(char) * 30);

	unsigned char *data = p.value;

	*(int *) data	= 10;
	data			+= sizeof(int);

	memcpy(data, "'Frenchie'", sizeof("'Frenchie'"));
	data			+= sizeof(char) * 20;

	memcpy(data, "'Minnie'", sizeof("'Minnie'"));
	data			+= sizeof(char) * 30;

	data			+= sizeof(int);

	memcpy(data, "'Penticton'", sizeof("'Penticton'"));
	return p;
}

void
setInt(
	iinq_prepared_sql	p,
	int					param_num,
	int					val
) {
	unsigned char	*value	= malloc(sizeof(int) + sizeof(char) * 20 + sizeof(char) * 30 + sizeof(int) + sizeof(char) * 30);
	unsigned char	*data	= value;

	data += sizeof(int) + sizeof(char) * 20 + sizeof(char) * 30;

	if (param_num == 3) {
		*(int *) data = val;
	}

	data += sizeof(int) + sizeof(char) * 30;

	memcpy(p.value, value, sizeof(&value));
	free(value);
}

void
execute(
	iinq_prepared_sql p
) {
	insert("Dogs.inq", IONIZE(10, int), p.value);

	free(p.value);
}
