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
	fremove("4.ffs");
	fremove("5.ffs");
	fremove("6.ffs");
	fremove("0.inq");
	fremove("1.inq");
	fremove("2.inq");
	fremove("3.inq");
	fremove("4.inq");
	fremove("ion_mt.tbl");
}

/* TODO: test OR and XOR once implemented */
int
main(
	void
) {
	/* Clean-up */
	cleanup();

	/* Test CREATE TABLE statement (table_id = 0)*/
/*  SQL_execute("CREATE TABLE Dogs (id INT, type CHAR(20), name VARCHAR(30), age INT, city VARCHAR(30), primary key(id));"); */
	create_table(0, key_type_numeric_signed, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 83));

	/* Test INSERT statements */
	printf("INSERT INTO Dogs VALUES (10, 'Frenchie', 'Minnie', 1, 'Penticton');\n");
/*  SQL_execute("INSERT INTO Dogs VALUES (10, 'Frenchie', 'Minnie', 1, 'Penticton');"); */
	iinq_execute_instantaneous(iinq_insert_0(10, "Frenchie", "Minnie", 1, "Penticton"));
	print_table(0);
	printf("INSERT INTO Dogs VALUES (40, 'Chihuahua', 'Barky', 7, 'Van');\n");
/*  SQL_execute("INSERT INTO Dogs VALUES (40, 'Chihuahua', 'Barky', 7, 'Van');"); */
	iinq_execute_instantaneous(iinq_insert_0(40, "Chihuahua", "Barky", 7, "Van"));
	print_table(0);
	printf("INSERT INTO Dogs COLUMNS (id, type, age) VALUES (30, 'Black Lab', 5);\n");
/*  SQL_execute("INSERT INTO Dogs COLUMNS (id, type, age) VALUES (30, 'Black Lab', 5);"); */
	iinq_execute_instantaneous(iinq_insert_0(30, "Black Lab", "", 5, ""));
	print_table(0);
	printf("INSERT INTO Dogs COLUMNS (id, type) VALUES (20, 'Black Lab');\n");
/*  SQL_execute("INSERT INTO Dogs COLUMNS (id, type) VALUES (20, 'Black Lab');"); */
	iinq_execute_instantaneous(iinq_insert_0(20, "Black Lab", "", NULL, ""));
	print_table(0);
	printf("INSERT INTO Dogs COLUMNS (city, name, id) VALUES ('West Bench', 'Corky', 50);\n");
/*  SQL_execute("INSERT INTO Dogs COLUMNS (city, name, id) VALUES ('West Bench', 'Corky', 50);"); */
	iinq_execute_instantaneous(iinq_insert_0(50, "", "Corky", NULL, "West Bench"));
	print_table(0);

	/* Test UPDATE statement */
	printf("UPDATE Dogs SET id = id-1, age = age * 10 WHERE name = 'Barky';\n");
/*  SQL_execute("UPDATE Dogs SET id = id-1, age = age * 10 WHERE name = 'Barky';"); */
	update(0, 1, 2, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_equal, "Barky")), IINQ_UPDATE_LIST(IINQ_UPDATE(1, 1, iinq_subtract, IONIZE(1, int)), IINQ_UPDATE(4, 4, iinq_multiply, IONIZE(10, int))));
	print_table(0);

	/* Test DELETE statement */
	printf("DELETE FROM Dogs WHERE id < 50 AND age >= 5;\n");
/*  SQL_execute("DELETE FROM Dogs WHERE id < 50 AND age >= 5;"); */
	delete_record(0, 2, IINQ_CONDITION_LIST(IINQ_CONDITION(4, iinq_greater_than_equal_to, IONIZE(5, int)), IINQ_CONDITION(1, iinq_less_than, IONIZE(50, int))));
	print_table(0);

	/* Test DROP TABLE statement */
	printf("DROP TABLE Dogs;\n");
/*  SQL_execute("DROP TABLE Dogs;"); */
	drop_table(0);

	/* Create Dogs table for further testing (table_id = 1)*/
	printf("CREATE TABLE Dogs (id VARCHAR(2), type CHAR(20), name VARCHAR(30), age INT, city VARCHAR(30), primary key(id));\n");
/*  SQL_execute("CREATE TABLE Dogs (id VARCHAR(2), type CHAR(20), name VARCHAR(30), age INT, city VARCHAR(30), primary key(id));"); */
	create_table(1, key_type_char_array, (sizeof(char) * 3), (sizeof(int) * 1) + (sizeof(char) * 86));

	/* Test prepared statements */
	printf("INSERT INTO Dogs VALUES ('1', 'Black Lab', 'Minnie', 5, 'Penticton'); (prepared)\n");

/*  iinq_prepared_sql *p1 = SQL_prepare("INSERT INTO Dogs VALUES ('1', (?), 'Minnie', (?), 'Penticton');"); */
	iinq_prepared_sql *p1 = iinq_insert_1("1", "", "Minnie", NULL, "Penticton");

	setParam(p1, 2, "Black Lab");
	setParam(p1, 4, IONIZE(5, int));
	execute(p1);
	print_table(1);

	/* Test that multiple tables simultaneously will not break functionality (table_id = 2)*/
	printf("CREATE TABLE Cats (id INT, name VARCHAR(30), age INT, primary key(id));\n");
/*  SQL_execute("CREATE TABLE Cats (id INT, name VARCHAR(30), age INT, primary key(id));"); */
	create_table(2, key_type_numeric_signed, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 31));

	printf("INSERT INTO Cats VALUES (6, 'Buttons', 2);\n");
/*  SQL_execute("INSERT INTO Cats VALUES (6, 'Buttons', 2);"); */
	iinq_execute_instantaneous(iinq_insert_2(6, "Buttons", 2));
	print_table(2);
	printf("INSERT INTO Cats VALUES (4, 'Mr. Whiskers', 4);\n");
/*  SQL_execute("INSERT INTO Cats VALUES (4, 'Mr. Whiskers', 4);"); */
	iinq_execute_instantaneous(iinq_insert_2(4, "Mr. Whiskers", 4));
	print_table(2);

	printf("INSERT INTO Cats VALUES (5, 'Minnie', 6); (prepared)\n");

/*  iinq_prepared_sql *p2 = SQL_prepare(""INSERT INTO Cats VALUES (5, ?, (?));""); */
	iinq_prepared_sql *p2 = iinq_insert_2(5, "", NULL);

	setParam(p2, 2, "Minnie");
	setParam(p2, 3, IONIZE(6, int));
	execute(p2);
	iinq_close_statement(p2);
	print_table(2);

	/* Test DELETE with multiple conditions */
	printf("DELETE FROM Cats WHERE id >= 5 AND id < 10 AND name != 'Minnie';\n");
/*  SQL_execute("DELETE FROM Cats WHERE id >= 5 AND id < 10 AND name != 'Minnie';"); */
	delete_record(2, 3, IINQ_CONDITION_LIST(IINQ_CONDITION(2, iinq_not_equal, "Minnie"), IINQ_CONDITION(1, iinq_less_than, IONIZE(10, int)), IINQ_CONDITION(1, iinq_greater_than_equal_to, IONIZE(5, int))));
	print_table(2);

	/* Reinsert rows that were deleted */
	printf("INSERT INTO Cats VALUES (6, 'Buttons', 2);\n");
/*	 SQL_execute("INSERT INTO Cats VALUES (6, 'Buttons', 2);"); */
	iinq_execute_instantaneous(iinq_insert_2(6, "Buttons", 2));
	print_table(2);

	/* Test UPDATE with multiple conditions */
	printf("UPDATE Cats SET age = age + 90 WHERE id >= 5 AND id < 10 AND name != 'Minnie';\n");
/*  SQL_execute("UPDATE Cats SET age = age + 90 WHERE id >= 5 AND id < 10 AND name != 'Minnie';"); */
	update(2, 3, 1, IINQ_CONDITION_LIST(IINQ_CONDITION(2, iinq_not_equal, "Minnie"), IINQ_CONDITION(1, iinq_less_than, IONIZE(10, int)), IINQ_CONDITION(1, iinq_greater_than_equal_to, IONIZE(5, int))), IINQ_UPDATE_LIST(IINQ_UPDATE(3, 3, iinq_add, IONIZE(90, int))));
	print_table(2);
	printf("UPDATE Cats SET age = 90 WHERE age < 5;\n");
/*  SQL_execute("UPDATE Cats SET age = 90 WHERE age < 5;"); */
	update(2, 1, 1, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_less_than, IONIZE(5, int))), IINQ_UPDATE_LIST(IINQ_UPDATE(3, 0, 0, IONIZE(90, int))));
	print_table(2);

	/* Test update with implicit fields */
	printf("UPDATE Cats SET age = 90, id = id+1, name = 'Chichi' WHERE age < 5;\n");
/*  SQL_execute("UPDATE Cats SET age = 90, id = id+1, name = 'Chichi' WHERE age < 5;"); */
	update(2, 1, 3, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_less_than, IONIZE(5, int))), IINQ_UPDATE_LIST(IINQ_UPDATE(3, 0, 0, IONIZE(90, int)), IINQ_UPDATE(1, 1, iinq_add, IONIZE(1, int)), IINQ_UPDATE(2, 0, 0, "Chichi")));
	print_table(2);
	printf("UPDATE Cats SET age = 90, id = id+1, name = 'Chichi' WHERE id >= 5 AND id < 10 AND name != 'Minnie';\n");
/*  SQL_execute("UPDATE Cats SET age = 90, id = id+1, name = 'Chichi' WHERE id >= 5 AND id < 10 AND name != 'Minnie';"); */
	update(2, 3, 3, IINQ_CONDITION_LIST(IINQ_CONDITION(2, iinq_not_equal, "Minnie"), IINQ_CONDITION(1, iinq_less_than, IONIZE(10, int)), IINQ_CONDITION(1, iinq_greater_than_equal_to, IONIZE(5, int))), IINQ_UPDATE_LIST(IINQ_UPDATE(3, 0, 0, IONIZE(90, int)), IINQ_UPDATE(1, 1, iinq_add, IONIZE(1, int)), IINQ_UPDATE(2, 0, 0, "Chichi")));
	print_table(2);

	printf("UPDATE Cats SET age = age + 5 WHERE age > 2;\n");
/*  SQL_execute("UPDATE Cats SET age = age + 5 WHERE age > 2;"); */
	update(2, 1, 1, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_greater_than, IONIZE(2, int))), IINQ_UPDATE_LIST(IINQ_UPDATE(3, 3, iinq_add, IONIZE(5, int))));
	print_table(2);

	printf("DELETE FROM Cats WHERE age >= 10;\n");
/*  SQL_execute("DELETE FROM Cats WHERE age >= 10;"); */
	delete_record(2, 1, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_greater_than_equal_to, IONIZE(10, int))));
	print_table(2);

	/* Test query */
	printf("SELECT id, name FROM Cats WHERE age < 10;\n");

/*  iinq_result_set *rs1 = SQL_select("SELECT id, name FROM Cats WHERE age < 10;"); */
	iinq_result_set *rs1 = iinq_select(2, sizeof(int) + (sizeof(char) * 31), 1, 2, IINQ_CONDITION_LIST(IINQ_CONDITION(3, iinq_less_than, IONIZE(10, int))), IINQ_SELECT_LIST(1, 2));

	while (iinq_next(rs1)) {
		printf("ID: %i,", iinq_get_int(rs1, 1));
		printf(" name: %s\n", iinq_get_string(rs1, 2));
	}

	iinq_close_result_set(rs1);

	printf("DROP TABLE Cats;\n");
/*  SQL_execute("DROP TABLE Cats;"); */
	drop_table(2);
	printf("DROP TABLE Dogs;\n");
/*  SQL_execute("DROP TABLE Dogs;"); */
	drop_table(1);

	/* Test tables with composite keys and different orderings for key fields (table_id = 3 & 4)*/
	printf("CREATE TABLE test1 (id1 INT, id2 INT, value CHAR(5), PRIMARY KEY(id1, id2));\n");
/*	 SQL_execute("CREATE TABLE test1 (id1 INT, id2 INT, value CHAR(5), PRIMARY KEY(id1, id2));"); */
	create_table(3, key_type_char_array, sizeof(int) + sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 6));
	printf("CREATE TABLE test2 (id1 INT, id2 INT, value CHAR(5), PRIMARY KEY(id2, id1));\n");
/*	 SQL_execute("CREATE TABLE test2 (id1 INT, id2 INT, value CHAR(5), PRIMARY KEY(id2, id1));"); */
	create_table(4, key_type_char_array, sizeof(int) + sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 6));

	/* Test that the keys are also set when using prepared statements */
/*	 p1 = SQL_prepare("INSERT INTO test1 COLUMNS (id1, id2) VALUES (?, ?);"); */
	p1	= iinq_insert_3(NULL, NULL, "");
/*	 p2 = SQL_prepare("INSERT INTO test2 COLUMNS (id1, id2) VALUES (?, ?);"); */
	p2	= iinq_insert_4(NULL, NULL, "");

	setParam(p1, 1, IONIZE(1, int));
	setParam(p1, 2, IONIZE(2, int));

	setParam(p2, 1, IONIZE(1, int));
	setParam(p2, 2, IONIZE(2, int));

	printf("INSERT INTO test1 COLUMNS (id1, id2) VALUES (1, 2); (prepared)\n");
	execute(p1);
	iinq_close_statement(p1);
	print_table(3);
	printf("INSERT INTO test2 COLUMNS (id1, id2) VALUES (1, 2); (prepared)\n");
	execute(p2);
	iinq_close_statement(p2);
	print_table(4);

	/* TODO: print keys */

	/* Test that duplicate keys cannot be inserted */
	printf("INSERT INTO test1 COLUMNS (id1, id2) VALUES (5, 3);\n");
/*	 SQL_execute("INSERT INTO test1 COLUMNS (id1, id2) VALUES (5, 3);"); */
	iinq_execute_instantaneous(iinq_insert_3(5, 3, ""));
	print_table(3);
	printf("INSERT INTO test1 COLUMNS (id1, id2) VALUES (5, 3);\n");
/*	 SQL_execute("INSERT INTO test1 COLUMNS (id1, id2) VALUES (5, 3);"); */
	iinq_execute_instantaneous(iinq_insert_3(5, 3, ""));
	print_table(3);

	printf("INSERT INTO test2 COLUMNS (id1, id2) VALUES (5, 3);\n");
/*	 SQL_execute("INSERT INTO test2 COLUMNS (id1, id2) VALUES (5, 3);"); */
	iinq_execute_instantaneous(iinq_insert_4(5, 3, ""));
	print_table(4);
	printf("INSERT INTO test2 COLUMNS (id1, id2) VALUES (5, 3);\n");
/*	 SQL_execute("INSERT INTO test2 COLUMNS (id1, id2) VALUES (5, 3);"); */
	iinq_execute_instantaneous(iinq_insert_4(5, 3, ""));
	print_table(4);

	/* Test an UPDATE that would violate the primary key constraint */
	printf("UPDATE test1 SET id1 = 1, id2 = 2 WHERE id1 = 5 AND id2 = 3;\n");
/*	 SQL_execute("UPDATE test1 SET id1 = 1, id2 = 2 WHERE id1 = 5 AND id2 = 3;"); */
	update(3, 2, 2, IINQ_CONDITION_LIST(IINQ_CONDITION(2, iinq_equal, IONIZE(3, int)), IINQ_CONDITION(1, iinq_equal, IONIZE(5, int))), IINQ_UPDATE_LIST(IINQ_UPDATE(1, 0, 0, IONIZE(1, int)), IINQ_UPDATE(2, 0, 0, IONIZE(2, int))));
	print_table(3);

	/* Test an UPDATE that updates a key field */
	printf("UPDATE test1 SET id1 = id+1;\n");
/*	 SQL_execute("UPDATE test1 SET id1 = id1+1;"); */
	update(3, 0, 1, IINQ_UPDATE_LIST(IINQ_UPDATE(1, 1, iinq_add, IONIZE(1, int))));
	print_table(3);

	printf("DROP TABLE test1;\n");
/*	 SQL_execute("DROP TABLE test1;"); */
	drop_table(3);
	printf("DROP TABLE test2;\n");
/*	 SQL_execute("DROP TABLE test2;\n"); */
	drop_table(4);

	/* Clean-up */
	cleanup();

	return 0;
}
