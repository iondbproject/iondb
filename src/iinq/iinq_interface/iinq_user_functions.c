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
SQL_Dogs(
	int		value_1,
	char	*value_2,
	char	*value_3,
	int		value_4,
	char	*value_5
) {
	iinq_prepared_sql p = { 0 };

	p.setParam			= setParam;
	p.execute			= execute;
	p.table				= malloc(sizeof(int));
	*(int *) p.table	= 0;
	p.value				= malloc(sizeof(int) + sizeof(char) * 20 + sizeof(char) * 30 + sizeof(int) + sizeof(char) * 30);

	unsigned char *data = p.value;

	p.key			= malloc(sizeof(int));
	*(int *) p.key	= value_1;

	*(int *) data	= value_1;
	data			+= calculateOffset(p.table, 1);

	memcpy(data, value_2, sizeof(char) * 20);
	data			+= calculateOffset(p.table, 2);

	memcpy(data, value_3, sizeof(char) * 30);
	data			+= calculateOffset(p.table, 3);

	*(int *) data	= value_4;
	data			+= calculateOffset(p.table, 4);

	memcpy(data, value_5, sizeof(char) * 30);

	return p;
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
					return sizeof(int) + (sizeof(char) * 20);

				case 3:
					return sizeof(int) + (sizeof(char) * 50);

				case 4:
					return (sizeof(int) * 2) + (sizeof(char) * 50);

				case 5:
					return (sizeof(int) * 2) + (sizeof(char) * 80);

				default:
					return 0;
			}
		}

		case 1: {
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

void
setParam(
	iinq_prepared_sql	p,
	int					field_num,
	void				*val
) {
	unsigned char *data = p.value;

	if (*(int *) p.table == 0) {
		if ((field_num == 1) || (field_num == 4)) {
			data			+= calculateOffset(p.table, (field_num - 1));
			*(int *) data	= (int) val;
		}

		if ((field_num == 2) || (field_num == 3) || (field_num == 5)) {
			data += calculateOffset(p.table, (field_num - 1));
			memcpy(data, val, sizeof(val));
		}
	}

	if (*(int *) p.table == 1) {
		if ((field_num == 1) || (field_num == 3)) {
			data			+= calculateOffset(p.table, (field_num - 1));
			*(int *) data	= (int) val;
		}

		if (field_num == 2) {
			data += calculateOffset(p.table, (field_num - 1));
			memcpy(data, val, sizeof(val));
		}
	}
}

void
execute(
	iinq_prepared_sql p
) {
	if (*(int *) p.table == 0) {
		insert("Dogs.inq", IONIZE(*(int *) p.key, int), p.value);
	}

	if (*(int *) p.table == 1) {
		insert("Cats.inq", IONIZE(*(int *) p.key, int), p.value);
	}

	free(p.value);
}

void
create_table2(
) {
	printf("%s\n\n", "CREATE TABLE Cats (id INT, name VARCHAR[30], age INT, primary key(id))");

	ion_err_t					error;
	ion_dictionary_t			dictionary;
	ion_dictionary_handler_t	handler;

	error = iinq_create_source("Cats.inq", 0, sizeof(int), sizeof(int) + sizeof(char) * 30 + sizeof(int));

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	dictionary.handler	= &handler;
	error				= iinq_open_source("Cats.inq", &dictionary, &handler);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}

	error = ion_close_dictionary(&dictionary);

	if (err_ok != error) {
		printf("Error occurred. Error code: %i\n", error);
		return;
	}
}

iinq_prepared_sql
SQL_Cats(
	int		value_1,
	char	*value_2,
	int		value_3
) {
	iinq_prepared_sql p = { 0 };

	p.setParam			= setParam;
	p.execute			= execute;
	p.table				= malloc(sizeof(int));
	*(int *) p.table	= 1;
	p.value				= malloc(sizeof(int) + sizeof(char) * 30 + sizeof(int));

	unsigned char *data = p.value;

	p.key			= malloc(sizeof(int));
	*(int *) p.key	= value_1;

	*(int *) data	= value_1;
	data			+= calculateOffset(p.table, 1);

	memcpy(data, value_2, sizeof(char) * 30);
	data			+= calculateOffset(p.table, 2);

	*(int *) data	= value_3;

	return p;
}
