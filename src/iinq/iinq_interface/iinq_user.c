/******************************************************************************/
/**
		@file		iinq_user.c
		@author		Dana Klamut, Kai Neubauer

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
cleanup(
) {
	fdeleteall();
	fremove("1.ffs");
	fremove("2.ffs");
	fremove("3.ffs");
	fremove("0.inq");
	fremove("1.inq");
	fremove("2.inq");
	fremove("255.inq");
	fremove("ion_mt.tbl");
}

/* TODO: test OR and XOR once implemented */
int
main(
	void
) {
	/* Clean-up */
	cleanup();

	/* Test CREATE TABLE statement */
/*  SQL_execute("CREATE TABLE Dogs (id INT, type CHAR(20), name VARCHAR(30), age INT, city VARCHAR(30), primary key(id));"); */
	create_table(0, key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 80));

	/* Test INSERT statements */
/*  SQL_execute("INSERT INTO Dogs VALUES (10, 'Frenchie', 'Minnie', 1, 'Penticton');"); */
	execute(insert_0(10, "Frenchie", "Minnie", 1, "Penticton"));
/*  SQL_execute("INSERT INTO Dogs VALUES (40, 'Chihuahua', 'Barky', 7, 'Van');"); */
	execute(insert_0(40, "Chihuahua", "Barky", 7, "Van"));

/*  SQL_execute("INSERT INTO Dogs COLUMNS (id, type, age) VALUES (30, 'Black Lab', 5);"); */
	execute(insert_0(30, "Black Lab", NULL_FIELD, 5, NULL_FIELD));
/*  SQL_execute("INSERT INTO Dogs COLUMNS (id, type) VALUES (20, 'Black Lab');"); */
	execute(insert_0(20, "Black Lab", NULL_FIELD, NULL_FIELD, NULL_FIELD));
/*  SQL_execute("INSERT INTO Dogs COLUMNS (city, name, id) VALUES ('West Bench', 'Corky', 50);"); */
	execute(insert_0(50, NULL_FIELD, "Corky", NULL_FIELD, "West Bench"));

	/* Test UPDATE statement */
/*  SQL_execute("UPDATE Dogs SET id = id-1, age = age * 10 WHERE name = 'Barky';"); */
	update(0, print_table_0, key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 80), 1, 2, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_equal, "Barky")), IINQ_UPDATE_LIST(IINQ_UPDATE(1, 1, iinq_subtract, 1), IINQ_UPDATE(4, 4, iinq_multiply, 10)));

	/* Test DELETE statement */
/*  SQL_execute("DELETE FROM Dogs WHERE id < 50 AND age >= 5;"); */
	delete_record(0, print_table_0, key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 80), 2, IINQ_CONDITION_LIST(IINQ_CONDITION(4, iinq_greater_than_equal_to, 5), IINQ_CONDITION(1, iinq_less_than, 50)));

	/* Test DROP TABLE statement */
/*  SQL_execute("DROP TABLE Dogs;"); */
	drop_table(0);

	/* Create Dogs table for further testing */
/*  SQL_execute("CREATE TABLE Dogs (id INT, type CHAR(20), name VARCHAR(30), age INT, city VARCHAR(30), primary key(id));"); */
	create_table(1, key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 80));

	/* Test prepared statements */
/*  iinq_prepared_sql p1 = SQL_prepare("INSERT INTO Dogs VALUES (10, (?), 'Minnie', (?), 'Penticton');"); */
	iinq_prepared_sql p1 = insert_1(10, PREPARED_FIELD, "Minnie", PREPARED_FIELD, "Penticton");

	setParam(p1, 2, "Black Lab");
	setParam(p1, 4, "5");
	execute(p1);

	/* Test that multiple tables simultaneously will not break functionality */
/*  SQL_execute("CREATE TABLE Cats (id VARCHAR(2), name VARCHAR(30), age INT, primary key(id));"); */
	create_table(2, key_type_char_array, (sizeof(char) * 2), (sizeof(int) * 1) + (sizeof(char) * 32));

/*  SQL_execute("INSERT INTO Cats VALUES ('6', 'Buttons', 2);"); */
	execute(insert_2("6", "Buttons", 2));
/*  SQL_execute("INSERT INTO Cats VALUES ('4', 'Mr. Whiskers', 4);"); */
	execute(insert_2("4", "Mr. Whiskers", 4));

/*  iinq_prepared_sql p2 = SQL_prepare("INSERT INTO Cats VALUES ('5', ?, (?));"); */
	iinq_prepared_sql p2 = insert_2("5", PREPARED_FIELD, PREPARED_FIELD);

	setParam(p2, 2, "Minnie");
	setParam(p2, 3, 6);
	execute(p2);

	/* Test DELETE with multiple conditions */
/*  SQL_execute("DELETE FROM Cats WHERE id >= 5 AND id < 10 AND name != 'Minnie';"); */
	delete_record(2, print_table_2, key_type_char_array, (sizeof(char) * 2), (sizeof(int) * 1) + (sizeof(char) * 32), 3, IINQ_CONDITION_LIST(IINQ_CONDITION(2, iinq_not_equal, "Minnie"), IINQ_CONDITION(1, iinq_less_than, 10), IINQ_CONDITION(1, iinq_greater_than_equal_to, 5)));

	/* Test UPDATE with multiple conditions */
/*  SQL_execute("UPDATE Cats SET age = age + 90 WHERE id >= 5 AND id < 10 AND name != 'Minnie';"); */
	update(0, print_table_0, key_type_char_array, (sizeof(char) * 2), (sizeof(int) * 1) + (sizeof(char) * 32), 3, 1, IINQ_CONDITION_LIST(IINQ_CONDITION(2, iinq_not_equal, "Minnie"), IINQ_CONDITION(1, iinq_less_than, 10), IINQ_CONDITION(1, iinq_greater_than_equal_to, 5)), IINQ_UPDATE_LIST(IINQ_UPDATE(3, 3, iinq_add, 90)));
/*  SQL_execute("UPDATE Cats SET age = 90 WHERE age < 5;"); */
	update(0, print_table_0, key_type_char_array, (sizeof(char) * 2), (sizeof(int) * 1) + (sizeof(char) * 32), 1, 1, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_less_than, 5)), IINQ_UPDATE_LIST(IINQ_UPDATE(3, 0, 0, 90)));

	/* Test update with implicit fields */
/*  SQL_execute("UPDATE Cats SET age = 90, id = id+1, name = 'Chichi' WHERE age < 5;"); */
	update(0, print_table_0, key_type_char_array, (sizeof(char) * 2), (sizeof(int) * 1) + (sizeof(char) * 32), 1, 3, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_less_than, 5)), IINQ_UPDATE_LIST(IINQ_UPDATE(3, 0, 0, 90), IINQ_UPDATE(1, 1, iinq_add, "1"), IINQ_UPDATE(2, 0, 0, "'Chichi'")));
/*  SQL_execute("UPDATE Cats SET age = 90, id = id+1, name = 'Chichi' WHERE id >= 5 AND id < 10 AND name != 'Minnie';"); */
	update(0, print_table_0, key_type_char_array, (sizeof(char) * 2), (sizeof(int) * 1) + (sizeof(char) * 32), 3, 3, IINQ_CONDITION_LIST(IINQ_CONDITION(2, iinq_not_equal, "Minnie"), IINQ_CONDITION(1, iinq_less_than, 10), IINQ_CONDITION(1, iinq_greater_than_equal_to, 5)), IINQ_UPDATE_LIST(IINQ_UPDATE(3, 0, 0, 90), IINQ_UPDATE(1, 1, iinq_add, "1"), IINQ_UPDATE(2, 0, 0, "'Chichi'")));

/*  SQL_execute("UPDATE Cats SET age = age + 5 WHERE age > 2;"); */
	update(0, print_table_0, key_type_char_array, (sizeof(char) * 2), (sizeof(int) * 1) + (sizeof(char) * 32), 1, 1, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_greater_than, 2)), IINQ_UPDATE_LIST(IINQ_UPDATE(3, 3, iinq_add, 5)));

/*  SQL_execute("DELETE FROM Cats WHERE age >= 10;"); */
	delete_record(2, print_table_2, key_type_char_array, (sizeof(char) * 2), (sizeof(int) * 1) + (sizeof(char) * 32), 1, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_greater_than_equal_to, 10)));

	/* Test query */
	/*iinq_result_set rs1 = SQL_select("SELECT id, name FROM Cats WHERE age < 10;");*/

	printf("sizeof value: %zu\n", (sizeof(int) * 2) + (sizeof(char) * 30));

/*  SQL_execute("DROP TABLE Cats;"); */
	drop_table(2);

	/* Clean-up */
	cleanup();

	return 0;
}
