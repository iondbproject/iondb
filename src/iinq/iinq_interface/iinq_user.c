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
cleanup(
) {
	fremove("1.ffs");
	fremove("2.ffs");
	fremove("3.ffs");
	fremove("6.ffs");
/*	fremove("3.ffs"); */
/*	fremove("4.ffs"); */
	fremove("Cats.inq");
	fremove("SEL.inq");
	fremove("DEL.inq");
	fremove("UPD.inq");
/*	fremove("Dogs.sch"); */
/*	fremove("Cust.inq"); */
/*	fremove("Cust.sch"); */
	fremove("ion_mt.tbl");
}

int
main(
	void
) {
	/* Clean-up */
	cleanup();

/*	  SQL_execute("INSERT INTO Dogs VALUES (10, 'Frenchie', 'Minnie', 1, 'Penticton');");  */
/*	  SQL_execute("INSERT INTO Dogs VALUES (40, 'Chihuahua', 'Barky', 7, 'Van');"); */
/*	  SQL_execute("INSERT INTO Dogs VALUES (30, 'Black Lab', 'Thunder', 5, 'Penticton');"); */
/*	  SQL_execute("INSERT INTO Dogs VALUES (50, 'Cockapoo', 'Corky', 2, 'West Bench');"); */
/*	  SQL_execute("UPDATE Dogs SET id = id-1, age = age * 10 WHERE name = 'Barky';"); */
/*	  SQL_execute("DELETE FROM Dogs WHERE age < 5;"); */
/*	  SQL_execute("DROP TABLE Dogs;"); */

/* / *	SQL_execute("CREATE TABLE Dogs (id INT, type CHAR[20], name VARCHAR[30], age INT, city VARCHAR[30], primary key(id));"); * / */
/*  */
/* / *   iinq_prepared_sql p1 = SQL_prepare("INSERT INTO Dogs VALUES (10, (?), 'Minnie', (?), 'Penticton');");  * / */
/*  */
/*	p1.setParam(p1, 2, "Bulldog"); */
/*	p1.setParam(p1, 4, (int *) 5); */
/*	p1.execute(p1); */

/*		  SQL_execute("CREATE TABLE Cats (id INT, name VARCHAR[30], age INT, primary key(id));"); */
	create_table("Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 30));

/*		  iinq_prepared_sql	p2	= SQL_prepare("INSERT INTO Cats VALUES (1, 'Chester', (?));"); */
/*			iinq_prepared_sql	p2	= insert_Cats(1, "'Chester'", NULL); */
/*	setParam(p2, 3, (int *) 4); */
/*	execute(p2); */

	iinq_prepared_sql p2 = insert_Cats(0, "'Beau'", 1);

	execute(p2);

	iinq_prepared_sql p3 = insert_Cats(1, "'Beau'", 2);

	execute(p3);

	iinq_prepared_sql p4 = insert_Cats(2, "'Beau'", 3);

	execute(p4);

	iinq_prepared_sql p5 = insert_Cats(3, "'Beau'", 4);

	execute(p5);

	iinq_prepared_sql p6 = insert_Cats(4, "'Beau'", 5);

	execute(p6);

	iinq_prepared_sql p7 = insert_Cats(5, "'Beau'", 6);

	execute(p7);

	iinq_prepared_sql p8 = insert_Cats(6, "'Beau'", 7);

	execute(p8);

	iinq_prepared_sql p0 = insert_Cats(7, "'Beau'", 8);

	execute(p0);

	iinq_prepared_sql p9 = insert_Cats(8, "'Beau'", 9);

	execute(p9);

	iinq_prepared_sql p11 = insert_Cats(9, "'Beau'", 10);

	execute(p11);

	iinq_prepared_sql p12 = insert_Cats(10, "'Beau'", 1);

	execute(p12);

/*	  iinq_prepared_sql	p3	= SQL_prepare("INSERT INTO Cats VALUES (2, (?), 88);"); */
/*	  iinq_prepared_sql	p3	= insert_Cats(2, "(?)", 88); */
/*	setParam(p3, 2, "'Beau'"); */
/*	execute(p3); */

/*		  SQL_execute("DELETE FROM Cats WHERE id >= 5, id < 10, name != 'Minnie';"); */
/*	delete_record(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2)+(sizeof(char) * 30), 9, 1, iinq_greater_than_equal_to, 5, 1, iinq_less_than, 10, 2, iinq_not_equal, "'Minnie'"); */

/*  SQL_execute("UPDATE Cats SET age = 90 WHERE age < 5;"); */
/*	update(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2)+(sizeof(char) * 30), 3, 4, 7, 3, iinq_less_than, 5, 3, 0, 0, 90); */

/*  SQL_execute("UPDATE Cats SET age = 90 WHERE id >= 5, id < 10, name != 'Minnie';"); */
/*	update(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2)+(sizeof(char) * 30), 9, 4, 13, 1, iinq_greater_than_equal_to, 5, 1, iinq_less_than, 10, 2, iinq_not_equal, "'Minnie'", 3, 0, 0, 90); */

/*  SQL_execute("UPDATE Cats SET age = 90, id = id+1, name = 'Chichi' WHERE age < 5;"); */
/*	update(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2)+(sizeof(char) * 30), 3, 12, 15, 3, iinq_less_than, 5, 3, 0, 0, 90, 1, 1, iinq_add, 1, 2, 0, 0, "'Chichi'"); */

/*  SQL_execute("UPDATE Cats SET age = 90, id = id+1, name = 'Chichi' WHERE id >= 5, id < 10, name != 'Minnie';"); */
/*	update(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2)+(sizeof(char) * 30), 9, 12, 21, 1, iinq_greater_than_equal_to, 5, 1, iinq_less_than, 10, 2, iinq_not_equal, "'Minnie'", 3, 0, 0, 90, 1, 1, iinq_add, 1, 2, 0, 0, "'Chichi'"); */

/*		  iinq_prepared_sql	p4	= SQL_prepare("INSERT INTO Cats VALUES (3, 'Buttons', 2);"); */
/*		  iinq_prepared_sql	p4	= insert_Cats(3, "'Buttons'", 2); */
/*	execute(p4); */

/*		  iinq_prepared_sql	p5	= SQL_prepare("INSERT INTO Cats VALUES (4, 'Mr. Whiskers', 4);"); */
/*		  iinq_prepared_sql	p5	= insert_Cats(4, "'Mr. Whiskers'", 4); */
/*	execute(p5); */

/*		  SQL_execute("UPDATE Cats SET age = age + 5 WHERE age > 2;"); */
/*  update(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2)+(sizeof(char) * 30), 3, 4, 7, 3, iinq_greater_than, 2, 3, 3, iinq_add, 5); */

/*		  SQL_execute("DELETE FROM Cats WHERE age >= 10;"); */
/*  delete_record(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2)+(sizeof(char) * 30), 3, 3, iinq_greater_than_equal_to, 10); */

/*	  iinq_result_set rs1 = SQL_select("SELECT id, name FROM Cats WHERE age < 10;"); */
	iinq_result_set rs1 = iinq_select(0, "Cats.inq", key_type_numeric_unsigned, sizeof(int), (sizeof(int) * 2) + (sizeof(char) * 30), 3, 2, 5, 3, iinq_less_than, 10, 1, 2);

	printf("sizeof value: %zu\n", (sizeof(int) * 2) + (sizeof(char) * 30));

	while (next(&rs1)) {
		printf("ID: %i,", getInt(&rs1, 1));
		printf(" name: %s\n", getString(&rs1, 1));
	}

/*  SQL_execute("DROP TABLE Cats;"); */
	drop_table("Cats.inq");

	/* Clean-up */
	cleanup();

	return 0;
}
